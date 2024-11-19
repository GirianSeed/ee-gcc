/*
PR 16213
We want to make sure that we can have a GIF tag inside an unpack
data block.  If pr16213.dvpasm assembles with exit code 0, we're OK.
However, the tcl code expects to build and execute something so
we'll let it build and execute this null program.  If the assembly
of the other file fails, the test will still fail.
*/
int main( int argc, void *argv[])
{
	return 0;
}
