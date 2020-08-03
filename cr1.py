import posix
import sunaudiodev
import socket


def doit(hn):


# open the sun audio stuff

  s = sunaudiodev.open('w')


  sock1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

  sock2.bind('', 9110)

# the 44 and 6 are octal that  correspond to 9222 in net byte order
  initstring = 'Cyber Radio 1'+ '\000' + '\44' +  '\6'
  username = 'selbie'
  while (len(username) < 20):
    username = username + '\000'

  hostname = 'toulouse'
  while (len(hostname) < 30):
    hostname = hostname + '\000'

  domainname = 'cc.gatech.edu'
  while (len(domainname) < 30):
    domainname = domainname + '\000'

  initstring = initstring + username + hostname + domainname
# 3 corresponds to REQUEST
  initstring = initstring + '\003'
  
  sock1.sendto(initstring, (hn, 19222));

  count = 0
  while (1):
    (aupack, hostadd) = sock2.recvfrom(1400)
    s.write(aupack[26:])
    count = count+1
    if (count > 123):
      sock1.sendto(initstring, (hn, 19222));
      count = 0;




doit('cyber.radio.org')

