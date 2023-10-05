import mmap
import time

MMAPNAME = "Global\\omnetSharedMemory"
BUFFSIZE = 200000

shm = mmap.mmap(0, BUFFSIZE, MMAPNAME)

def printParsedResult(string):
    temp = string.split('/')
    for line in temp:
        print(line)

def returnFinedResult(strResult):
    finedResult = ""
    for char in strResult[1:]:
        if char != "!": finedResult += char
        else: break

    return finedResult

def returnCommandString(commandList):
    string = ""
    for command in commandList:
        temp = command.split(' ')
        string += "{} {}/".format(temp[0], temp[1])

    return string

def sendOmnetCommand(myCommand):
    myCommand = "p" + myCommand + "!"
    byteMessage = myCommand.encode()
    shm.write(byteMessage)


if shm:
    while True:
        shm.seek(0)
        byteResult = shm.readline()
        strResult = byteResult.decode() # mmap에 저장돼있는 명령 구문 저장 
        
        if strResult.replace("\x00", '') != '': # 메모리가 처음 생성된게 아니라면
            if strResult[0] == 'c': # omnet의 명령이면
                print("====================================================================================")
                finedResult = returnFinedResult(strResult)
                #printParsedResult(finedResult) # 네트워크 정보 출력
                print(finedResult)
                
                shm.seek(0)

                
                sendOmnetCommand("hello") # 입력 끝나면 omnet에 전송

        else: # 메모리가 처음 생성되었다면
            shm.seek(0)
            byteMessage = "pf!".encode()
            shm.write(byteMessage)
            print("첫 번째 연결, 메모리 초기화 완료")
                

else:
    print("공유 메모리 접근 불가")
'''
i = 0
while True:
    shm.seek(0)
    string = str(i).encode()
    shm.write(string)
    i += 1
    time.sleep(1)
'''
