
import sys
import os

def crawl(dirname):
    l = []
    for d in os.listdir(dirname):
        p = os.path.abspath(dirname)
        path = os.path.join(p, d)
        with open(path, "r", errors="replace") as f:
            print(f"analyzing {path}")
            url = f.readline().strip()
            depth = f.readline()
            l.append([int(d), url, int(depth)])
    
    l.sort(key=lambda x: x[0])

    for e in l:
        print(f"id:{e[0]}, {e[1]}, depth:{e[2]}")

    print(f"TOTAL: {len(l)}")

def main(argv):
    crawl(argv[1])



if __name__ == "__main__":
    main(sys.argv)