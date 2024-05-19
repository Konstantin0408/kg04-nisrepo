all: TCPEchoClientLoop TCPEchoServer
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c ClientBehaviour.c -o TCPEchoClientLoop
TCPEchoServer: TCPEchoServer.c DieWithError.c
	gcc TCPEchoServer.c DieWithError.c HandleTCPClient.c -o TCPEchoServer
