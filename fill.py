import sys

def main():
    if len(sys.argv) < 3:
        print("Usage: python fill.py <file> <n>")
        exit(-1)
    f = open(sys.argv[1], "w+")
    n = int(sys.argv[2])
    f.write(str(n) + "\n")
    for i in range(n):
        f.write(" ".join(list(map(lambda x: str(x % n), list(range(i, i + n))))) + "\n")
    for i in range(n):
            f.write(" ".join(list(map(lambda x: str(x % n), list(range(i + (n // 2), i + (n // 2) + n))))) + "\n")
    f.close()

if __name__ == "__main__":
    main()
