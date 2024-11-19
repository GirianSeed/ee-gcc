# Globals

global CGENPL
set CGENPL $srcdir/$subdir/c_gen.pl

global L2GENPL
set L2GENPL $srcdir/$subdir/level2_gen.pl

global DVPAS
if ![info exists DVPAS] {
    set DVPAS [findfile $base_dir/../../dvp-gas/as-new \
		   $base_dir/../../dvp-gas/as-new \
		   ee-dvp-as]
}

global DVPASFLAGS
if ![info exists DVPASFLAGS] {
    set DVPASFLAGS ""
}


# find a perl with version >= $min_version
# first searches $env(PATH),
# then searches the dirs given as $args
proc findperl {min_version args} {
    global env

    # FIXME: unix centric
    set pathsep {:}

    foreach dir [concat [split $env(PATH) $pathsep] $args] {
	foreach perl {perl perl5 perl4} {
	    set path [file join $dir $perl]

	    if ![file executable $path] {
		continue
	    }
#	    if [catch {set ver [exec -- $path -e "print $\]"]}] {
#		continue
#	    }
#	    if ![regexp {^[0-9]+[.][0-9]*} $ver] {
#		continue
#	    }
#	    if {$ver < $min_version} {
#		continue
#	    }

	    return $path
	}
    }
    error [concat {Can't find perl >= } \
	       $min_version \
	       {in PATH or} \
	       [list $args]]
}


global PERL
set PERL [findperl 5.0 /usr/bin /usr/clocal/bin /usr/local/bin]


# Procedures


# Run a .trc file.
# Assert success and matching vif1expect/vif0expect/gifexpect/gsexpect
# (if exists)

proc run_trc_test { trc } {
    global CGENPL PERL
    global subdir srcdir
    global timeout
 
    #save old timeout value (in order to restore at the end):
    set oldtimeout $timeout
    
    set timeout 500

    set mach "sky"
    regsub ".trc$" [file tail $trc] "" test
    set junk_array ""
   
    verbose "Testing $trc on $mach"

    # Convert .trc -> .c
    regsub "\.trc$" $trc ".c" cfile
    set cfile [file tail $cfile]
    send_log "$PERL $CGENPL $trc $cfile\n"
    catch "exec $PERL $CGENPL $trc $cfile" cgenpl_out
    if [string match "" $cgenpl_out] {
	verbose -log "$cgenpl_out" 3
	perror "$PERL $CGENPL $trc failed"
    }
    lappend junk_array $cfile

    # Compile .c
    regsub "\.c$" $cfile ".run" runfile
    set runfile [file tail $runfile]
    if {[sim_compile $cfile $runfile executable "debug"] != ""} {
	fail "$mach $test"
	return
    }
    lappend junk_array $runfile

    set cmp_array ""
    set run_suffix ""
    #setup default run_flags:
    set run_flags "--enable-gs off"
    
    foreach exp [lsort [glob -nocomplain $srcdir/$subdir/$test.*expect]] {
	if [string match "*.vif1expect" $exp] {
	    regsub "vif1expect$" $exp "vif1out" res
	    set res [file tail $res]
	    set run_flags "$run_flags --sky-debug vif1 --sky-debug-file vif1=$res"
	    lappend cmp_array "diff $res $exp"
	    lappend junk_array $res
	    verbose -log "$test VIF1" 4 
	}
	if [string match "*.vif0expect" $exp] {
	    regsub "vif0expect$" $exp "vif0out" res
	    set res [file tail $res]
	    set run_flags "$run_flags --sky-debug vif0 --sky-debug-file vif0=$res"
	    lappend cmp_array "diff $res $exp"
	    lappend junk_array $res
	    verbose -log "$test VIF0" 4
	}  
        if [string match "*.gifexpect" $exp] {
	    set res "$test.gifout"
	    set run_flags  " --enable-gs off"
            set run_suffix " > $res"
	    lappend cmp_array "diff -bitw $res $exp"
 	    lappend junk_array $res
        }
        if [string match "*.gsexpect" $exp] {
	    set res "$test.gsout"
	    set run_flags  " --enable-gs off"
	    set run_suffix " > $res"
	    lappend cmp_array "diff -bitw $res $exp"
 	    lappend junk_array $res
        }

	# add other comparisons here
    }

    # check for xEXPECT files

    # Run test case

    set result [sim_run $runfile $run_flags "" $run_suffix "timeout=$timeout"]
    set status [lindex $result 0]
    set output [lindex $result 1]

    if [is_remote host] {
	if { [remote_upload host $res] != "" } {
	    remote_file host delete $res
	}
    }

    if { "$status" == "pass" } {
	foreach cmptest $cmp_array {
	    send_log "$cmptest\n"
	    set rc [catch "exec $cmptest" cmp_out]
	    if {$rc != 0} {
		verbose -log "$mach $test $cmptest rc=$rc" 2 
		verbose -log "$cmp_out" 2 
		set status fail
	    }
	}
    }

    # Clean garbage, but only if we passed.
    if { "$status" == "pass" } {
	foreach junk $junk_array {
	    file delete $junk
	}
    }

    $status "$mach $test"

    #restore the old timeout value
    set timeout $oldtimeout
    verbose "Timeout is restored to $timeout" 1 
   
}


# Build & run a more complex test case.  Compile & assemble pieces specified in
# given .brn file, assert success
proc run_brn_test { brn } {
    global CGENPL AS ASFLAGS DVPAS DVPASFLAGS
    global subdir srcdir
    global timeout
     
    #save old timeout value (in order to restore at the end):
    set oldtimeout $timeout

    # Default value of timeout
    set timeout 500

    # preset variables
    set mach "sky"
    set c_files ""
    set c_flags ""
    set dvpasm_files ""
    set dvpasm_flags ""
    set asm_files ""
    set asm_flags ""
    set ld_flags ""
    set run_file ""
    set run_flags ""
    set run_suffix ""
    set test_commands ""
    set incasefail_commands ""
    set output_files ""
    set junk_files ""
    set status running
    # Set to true if testcase is supported on skyb.
    set test_on_skyb_p 1
    
    regsub ".brn$" [file tail $brn] "" test
    
    # allow overriding of variables
    source $brn

    # If this is skyb, pass -no-dma and tell sce_macros to ignore .section.
    if [istarget *-skyb-*] {
	if { !$test_on_skyb_p } {
	    verbose -log "$brn not supported on skyb"
	    return
	}
	append dvpasm_flags "-no-dma --defsym skyb_p=1"
    } else {
	append dvpasm_flags "--defsym skyb_p=0"
    }

    verbose "Testing $brn on $mach"

    # xfail certain tests for PR 17743:
    if {-1 != [lsearch -exact [list sce2_test11 sce2_test12 sce2_test13 sce2_test23 sce_test13 sce_test14 sce_test15 sce_test16 sce_test17 sce_test18 sce_test19 sce_test20 sce_test21 sce_test22 sce_test24 sce_test25 sce_test26 sce_test27 sce_test28 sce_test29 sce_test30 sce_test31 sce_test32 sce_test33 sce_test34 sce_test45 sce_test46 sce_test47 sce_test48 sce_test52 sce_test53 sce_test54] $test]} {
	setup_xfail *
    }
    
    # timeout maybe one the values reset by *brn:
    verbose "$mach test: timeout is $timeout" 1
    
    # compile C files
    foreach cfile $c_files {
	regsub "\.c$" $cfile ".o" objfile
	set objfile [file tail $objfile]

	lappend obj_files $objfile
	if {[sim_compile $cfile $objfile object "{additional_flags=-I$srcdir/$subdir $c_flags}"] != ""} {
	    warning "$mach $test - compile"
	}
    }

    # assemble S files
    foreach asmfile $asm_files {
	regsub "\.s$" $asmfile ".o" objfile
	set objfile [file tail $objfile]

	lappend obj_files $objfile
	send_log "$AS $ASFLAGS $asm_flags -I$srcdir/$subdir -o $objfile $asmfile\n"
	catch "exec $AS $ASFLAGS $asm_flags -I$srcdir/$subdir -o $objfile $asmfile" as_out
	if ![string match "" $as_out] {
	    verbose -log "$as_out" 3
	    warning "$mach $test - assemble"
	}
    }

    # assemble DVPASM files
    foreach dvpfile $dvpasm_files {
	if [string match "*dvpasm" $dvpfile] {
	    regsub "\.dvpasm$" $dvpfile "dvp.o" objfile
	} else {
	    regsub "\.vuasm$" $dvpfile "vu.o" objfile
	}
	set objfile [file tail $objfile]

	lappend obj_files $objfile
	send_log "$DVPAS $ASFLAGS $dvpasm_flags -I$srcdir/$subdir -o $objfile $dvpfile\n"
	catch "exec $DVPAS $ASFLAGS $dvpasm_flags -I$srcdir/$subdir -o $objfile $dvpfile" as_out
	if ![string match "" $as_out] {
	    verbose -log "$as_out" 3
	    warning "$mach $test - assemble"
	}
    }

    if [is_remote host] {
	foreach file $obj_files {
	    if { [remote_download host $file] == "" } {
		perror "download $file"
		unresolved "$mach $test"
		return
	    }
	}
    }

    # link object files
    if {[sim_compile $obj_files $run_file executable "debug {ldflags=$ld_flags}"] != ""} {
	set status fail
    }

    # Run test case
    if { "$status" == "running" } {
	set result [sim_run $run_file $run_flags "" $run_suffix "timeout=$timeout"]
	set status [lindex $result 0]
	set output [lindex $result 1]
        verbose -log "After sim_run: status is $status" 2

	if [is_remote host] {
	    foreach file $output_files {
		remote_upload host $file
	    }
	}

	if { "$status" == "pass" } {
	    foreach testcmd $test_commands {
		send_log "$testcmd\n"
		set rc [catch "exec $testcmd" testcmd_out]
		if {$rc != 0} {
		    verbose -log "$mach $test $testcmd rc=$rc" 2 
		    verbose -log "testcmd_out is $testcmd_out, status set to fail" 2  
		    set status fail
		}
	    }	   
	}
    }

    if { "$status" == "pass" } {
	# Clean up objects & run file
	foreach junk $obj_files {
	    file delete $junk
	}
	foreach junk $junk_files {
	    file delete $junk
	}
	file delete $run_file
    }
    
    if { "$status" == "fail" } {
        foreach Fcmd $incasefail_commands {
	  send_log "$Fcmd \n"
          set rc [catch "exec $Fcmd" Fcmd_out]
	  if {$rc != 0} {
	    verbose -log "${Fcmd_out}" 2  
	  }
	}
    }

    $status "$mach $test"

    #restore the old timeout value
    set timeout $oldtimeout
    verbose "Timeout is restored to $timeout" 1 
}



# Run a .l2s file.
# Assert success from level2_gen C mainline

proc run_l2s_test { l2s } {
    global L2GENPL PERL
    global subdir srcdir
    global timeout
    
    set oldtimeout $timeout
    #set timeout 120
    set timeout 1200

    set mach "sky"
    regsub ".s$" [file tail $l2s] "" test
    set junk_array ""

    verbose "Testing $l2s on $mach"

    # xfail certain tests for PR 17743:
    if {-1 != [lsearch -exact [list vpi10bypass_a1 vpi10efu_211 vpi10efu_212 vpi10efu_213 vpi10efu_215 vpi10efu_217 vpi10efu_218 vpi10efu_219 vpi10efu_21a vpi10efu_21b vpi10efu_21c vpi10efu_21d vpi10fmac_11] $test]} {
	setup_xfail *
    }

    # Convert .s -> .c
    regsub "\.s$" $l2s ".c" cfile
    set cfile [file tail $cfile]

    # TODO: code generation is hard-coded for vu0/vu1 based on
    # partial file name.  This test is not correct.  It may
    # be better to run all tests on both VUs.
    set vu 0
    regsub "_.*$" [file tail $l2s] "" l2class
    verbose -log "$l2class" 1
    set vu 1
    if [string match "*efu*" $l2class] { set vu 1 }
    if [string match "*data*" $l2class] { set vu 1 }
    if [string match "*fmac*" $l2class] { set vu 1 }
    if [string match "*fdiv*" $l2class] { set vu 1 }

    send_log "$PERL $L2GENPL $l2s $vu > $cfile\n"
    catch "exec $PERL $L2GENPL $l2s $vu > $cfile" l2genpl_out
    if ![string match "" $l2genpl_out] {
	verbose -log "$l2genpl_out" 1
	perror "$PERL $L2GENPL $l2s"
	return
    }
    lappend junk_array $cfile

    # collect any other junk files left over by level2_gen.pl
    foreach junkfile [glob -nocomplain "$test.*"] {
	lappend junk_array $junkfile
    }

    # Compile .c
    regsub "\.c$" $cfile ".run" runfile
    set runfile [file tail $runfile]
    if {[sim_compile $cfile $runfile executable "debug"] != ""} {
	perror "compiling $cfile - $mach $test"
	return
    }
    lappend junk_array $runfile

    set cmp_array ""
    set run_suffix ""
    #setup default run_flags:
    set run_flags "--enable-gs off --float-type accurate"
    
    # Run test case

    set result [sim_run $runfile $run_flags "" $run_suffix "timeout=$timeout"]
    set status [lindex $result 0]
    set output [lindex $result 1]

    # Clean garbage, but only if we passed.
    if { "$status" == "pass" } {
	foreach junk $junk_array {
	    file delete $junk
	}
    }

    set timeout $oldtimeout
    $status "$mach $test"
}
