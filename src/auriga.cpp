#include <stdio.h>
#include <stdlib.h>
#include "Message.h"
#include "Perft.h"

int main(int argc, const char* argv[])
{

	Message* message = new Message("wwwwwwwwwwww", 1, 1,1, "ssssssssss", 0, 1, -1, -1);
	message->print();
	cout <<endl;
	Perft &perft = Perft::getInstance();
	perft.status();
	//while(1);
	return 0;
}
