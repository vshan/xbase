import socket
import sys

mydict = {}

def loadDict():
  content = []
  with open("nameserverdict") as f:
    content = f.readlines()
    content = [x.strip('\n') for x in content]
  for line in content:
    strs = line.split(',')
    mydict[strs[0]] = (strs[1], strs[2])

def spawnServer():
  serverSocket = socket.socket()
  port = 7005
  host="127.0.0.1"
  serverSocket.bind((host,port))
  serverSocket.listen(5)
  print('Server listening...')
  max_length=10240

  while True:
    conn, addr=serverSocket.accept()
    print('connection received')
    data=conn.recv(max_length);
    print(data)
    res=handleReq(data.decode())
    conn.send(res.encode())
    # conn.close

def handleReq(req):
  # 70|FileName
  # 71|Port|FileName|IP
  # 90|FileName|Port|IP
  # 91|FileName
  strs = req.split('|')
  code = int(strs[0])
  filename = strs[1]
  if (code == 70):
    tup = mydict[filename]
    resultstr = str(71) + "|" + tup[1] + "|" + filename + "|" + tup[0]
    print(resultstr)
    return resultstr
  if (code == 90):
    port = strs[2]
    ip = strs[3]
    mydict[filename] = (ip, port)
    with open("nameserverdict", "a") as myfile:
      myfile.write(filename + "," + ip + "," + port + "\n")
    retrieveQuery("100|" + filename, ip, port)
    return str(91) + "|" + filename

def retrieveQuery(query, vinayb_ip, port):
  max_length=102400
  clientSocket=socket.socket()
  clientSocket.connect((vinayb_ip,int(port)))
  clientSocket.send(query.encode())
  data=clientSocket.recv(max_length)
  print(data)
  clientSocket.close
  return data.decode()

if __name__== "__main__" :
  loadDict()
  spawnServer()
