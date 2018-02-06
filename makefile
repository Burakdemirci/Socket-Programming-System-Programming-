All:
	gcc seeWhat.c -o seeWhat -lm -w
	gcc timerServer.c -o timerServer -lm -w
	gcc showResult.c -o showResult -w

clean:
	rm *.o seeWhat timerServer showResult
