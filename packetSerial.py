'''
 Basic packet format
   STX
   LEN
   FSEP
   DATA[LEN]
   FSEP
   CRC
   ETX
'''

import serial
from time import sleep
import binascii

LOG_DEBUG=False
LOG_WARNING=False

def logDebug( msg ):
    if (LOG_DEBUG==True):
        print msg

def logWarning( msg ):
    if (LOG_WARNING==True):
        print msg


class PacketSerial:
    
    STX  = '<'
    FSEP = ','
    ETX  = '>'
    
    RX_STATE_READING_STX      = 1
    RX_STATE_READING_LEN_DATA = 2
    RX_STATE_READING_DATA     = 3    
    RX_STATE_READING_CRC      = 4
    RX_STATE_READING_ETX      = 5
        
    def __init__( self, serial):
        '''
        '''
        self.serial = serial
        
        self.data          = []
        self.dataAvailable = False

        self.resetToStartState()

    def resetToStartState( self ):
        '''
        '''
        self.receiveState       = self.RX_STATE_READING_STX
        self.receiveBuffer      = ""        
        
        self.receiveDataLen     = None        
        self.receiveData        = None        
        self.receiveCRC         = None        

    def sendData( self, data ):
        '''
        '''
        
        # build packet and calculate CRC        
        content = ''
        
        # length of data element
        content += str( len(data) )
        content += self.FSEP
               
        # data element        
        content += data
        
        # CRC    
        crc = str ( self.calcCRC(data) )
                
        packet =  self.STX
        packet += content
        packet += self.FSEP
        packet += crc
        packet += self.ETX

        logDebug("Sending: %s" % packet)

        # The Arduino only has a serial ring buffer of 64 bytes
        # When sending large packets of data this can result in dropped 
        # characters so here we limit the size of the chunks we send
        # and add a small gap in between them to work around this limitation

        MAX_CHUNK_SIZE = 48
        INTER_CHUNK_DELAY = 0.2

        if (len(packet)<=MAX_CHUNK_SIZE): 
            self.serial.write(packet)
            self.serial.flush()
        else:
            def chunks(l, n):
                n = max(1, n)
                return (l[i:i+n] for i in xrange(0, len(l), n))

            for chunk in chunks(packet, MAX_CHUNK_SIZE):
                self.serial.write(chunk)
                self.serial.flush()
                sleep(INTER_CHUNK_DELAY)
                                        
    def getData( self ):            
        '''
        '''
        if (self.dataAvailable==True):
            rc = self.data
            self.dataAvailable = False
            self.data = []
            return rc
        else:
            self.processIncoming()        
            return None
                                    
    def processIncoming( self ):
        '''
        Read all incoming characters until we have read a packet or buffer is empty
        '''

        rxChar = self.getNextChar()

        # loop until we have read a packet or there are no incoming chars left
        while ((self.dataAvailable==False) and (rxChar is not None)):

            if (rxChar is not None):

                # *****************************************************
                # STX waiting for packet start
                # *****************************************************
                if (self.receiveState==self.RX_STATE_READING_STX):
                    logDebug('RX_STATE_READING_STX:%s' % rxChar)

                    if (rxChar==self.STX):
                        # found packet start
                        self.receiveBuffer=""
                        self.receiveState=self.RX_STATE_READING_LEN_DATA
                    else:
                        # not packet start so ignore the char
                        #   maybe we started reading mid packet
                        logWarning('RX_STATE_READING_STX: Encountered unexpected char in stream: [%s]' % rxChar)

                # *****************************************************
                # reading data len
                # *****************************************************
                elif (self.receiveState==self.RX_STATE_READING_LEN_DATA):
                    logDebug('RX_STATE_READING_LEN_DATA:%s' % rxChar)

                    if (rxChar!=self.FSEP):
                        nums = [ str(x) for x in xrange(0,10) ]
                        if (rxChar in nums):
                            self.receiveBuffer += rxChar
                        else:
                            logWarning('RX_STATE_READING_LEN_DATA: Encountered unexpected char in stream: [%s]' % rxChar)
                            self.resetToStartState()

                    else:
                        self.receiveDataLen = int(self.receiveBuffer)
                        self.receiveData    = []
                        self.receiveBuffer  = ""
                        self.receiveState   = self.RX_STATE_READING_DATA

                # *****************************************************
                # reading data
                # *****************************************************
                elif (self.receiveState==self.RX_STATE_READING_DATA):
                    logDebug('RX_STATE_READING_DATA:%s' % rxChar)

                    if (len(self.receiveBuffer)< self.receiveDataLen):

                        self.receiveBuffer += rxChar

                    else:
                        # should be field seperator after data element
                        if (rxChar==self.FSEP):
                            self.receiveData = self.receiveBuffer
                            self.receiveBuffer = ""
                            self.receiveState=self.RX_STATE_READING_CRC

                        else:
                            # was expecting field seperator after data block
                            logWarning('RX_STATE_READING_DATA: Encountered unexpected char in stream: [%s]' % rxChar)
                            self.resetToStartState()

                # *****************************************************
                # reading CRC
                # *****************************************************
                elif (self.receiveState==self.RX_STATE_READING_CRC):
                    logDebug('RX_STATE_READING_CRC:%s' % rxChar)

                    # read all numbers into CRC
                    nums = [ str(x) for x in xrange(0,10) ]
                    if (rxChar in nums):
                        self.receiveBuffer += rxChar

                    # until we hit ETX
                    elif (rxChar==self.ETX):

                        self.receiveCRC = int(self.receiveBuffer)
                        self.receiveBuffer = ""

                        if ( self.checkCRC(self.receiveData, self.receiveCRC) == True):
                            self.data = self.receiveData
                            self.dataAvailable = True
                            self.resetToStartState()
                        else:
                            logWarning('RX_STATE_READING_CRC: CRC check failed')
                            self.resetToStartState()

                    else:
                        logWarning('RX_STATE_READING_CRC: Encountered unexpected char in stream: [%s]' % rxChar)
                        self.resetToStartState()

                else:
                    pass

            rxChar = self.getNextChar()


    def getNextChar( self ):
        '''
        '''
        rc = None

        if (self.serial.inWaiting() > 0):
            rc = self.serial.read(1)

        return rc

    def calcCRC( self, content ):
        #crc = binascii.crc32( str( content) ) & 0xffffffff
        return 0

    def checkCRC( self, content, crc ):
        calcCRC = self.calcCRC( content )
        
        if (calcCRC != crc ):
            return False
        else:
            return True    



import sys

def run():
    '''
    Reads packets from the serial port and prints to console
    Reads lines from console and sends to serial port as a packet
    '''

    portName = 'COM5'
    print 'Using port %s' % portName

    serialPort = serial.Serial( portName, 57600 )
    ps = PacketSerial(serialPort)

    while (True):

        # Get some data and send it

        print "Enter some data to send and press return"
        input = sys.stdin.readline()
        input = input[:len(input)-1]     # strip CR/NL
        ps.sendData(input)

        # Read all of the packets received and display

        rxData = ps.getData()
        while (rxData is not None):
            print "Packet received; data = [%s]" % rxData
            rxData = ps.getData()

    sleep(0.01)

if __name__ == '__main__':
    #LOG_DEBUG=True
    #LOG_WARNING=True
    run()



