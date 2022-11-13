echo "[1] building" && make testParser LDFLAGS="-c" && echo "[2] linking" && g++ parser -lLLVM-15 && echo "[3] running" && ./a.out xc.c
