import win32pipe, win32file, pywintypes
import json
from DotDict import DotDict

class Communicator(Exception):

    def __init__(self, pipe_name, buffer_size):
        self.pipe_name = pipe_name
        self.buffer_size = buffer_size
        self.pipe = self.init_pipe(self.pipe_name, self.buffer_size)

    # 통신 관련 함수
    def send_omnet_message(self, msg):
        win32file.WriteFile(self.pipe, msg.encode('utf-8'))
        
    def get_omnet_message(self):
        response_byte = win32file.ReadFile(self.pipe, self.buffer_size)
        response_str = response_byte[1].decode('utf-8')
        return response_str

    def close_pipe(self):
        win32file.CloseHandle(self.pipe)


    # 통신 관련 초기화
    def init_pipe(self, pipe_name, buffer_size):
        pipe = None
        try:
            pipe = win32pipe.CreateNamedPipe(
                pipe_name,
                win32pipe.PIPE_ACCESS_DUPLEX,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                1,
                buffer_size,
                buffer_size,
                0,
                None
            )
        except:
            print("예외 발생")
            return -1

        win32pipe.ConnectNamedPipe(pipe, None)

        return pipe

        




