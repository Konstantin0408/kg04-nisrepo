all: TCPEchoClientLoop TCPEchoServer TCPDisplayer
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c ClientBehaviour.c
	gcc TCPEchoClientLoop.c DieWithError.c ClientBehaviour.c -o TCPEchoClientLoop
TCPDisplayer: TCPDisplayer.c DieWithError.c
	gcc TCPDisplayer.c DieWithError.c -o TCPDisplayer
TCPEchoServer: TCPEchoServer.c DieWithError.c HandleTCPClient.c
	gcc TCPEchoServer.c DieWithError.c HandleTCPClient.c -o TCPEchoServer
