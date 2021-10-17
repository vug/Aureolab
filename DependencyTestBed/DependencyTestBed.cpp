// Forward decleration of Main functions of each example app
// Code for each app put into their own namespace so that 
namespace ObjectSelection { int Main(); }
namespace UniformBuffers { int Main(); }

int main(int argc, char* argv[]) {
	int result = 0;
	//result = UniformBuffers::Main();
	result = ObjectSelection::Main();
	return result;
}