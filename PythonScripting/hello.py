import time


def testFunc():
    from pathlib import Path
    p = Path(__file__).with_name('test.txt')
    with p.open('a') as f:
        f.write("appended text")
        #print(f.read())  

    print("Hi All")  
    return 'HiAll !!!'

def waitFunc():
    #time.sleep(4)
    print("Hi All")
    time.sleep(2)
    print("Hi All a second time")
    return "maybe"

if __name__ == '__main__':
    print("test_Main")
    testFunc()