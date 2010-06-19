all: ../ebin/ora.boot

clean:
	rm ../ebin/ora.{boot,script}

../ebin/ora.boot: ora.rel
	erlc -o ../ebin -pa ../ebin ora.rel

