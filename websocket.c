#include <p30f4013.h>
#include "websocket.h"
#include "sha1.h"
#include "base64.h"

extern unsigned char isConnected;               //debugging

unsigned char WebSocketKeyIdent[] = "Sec-WebSocket-Key: ";
char WebSocketKey[] = "dGhlIHNhbXBsZSBub25jZQ==";
BYTE WebSocketGuid [] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
BYTE ServerReply [] =
"HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"                         
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: ";
unsigned int keycntr = 0;

unsigned char wsData[255];
unsigned char wsByteCount;
unsigned int payloadlen = sizeof(wsData);

unsigned char cntrCRLF = 0;

extern unsigned char rxBuffer[];
extern unsigned char rxPtr;
unsigned char readPtr = 0;

extern unsigned char txBuffer[];
extern unsigned char txPtr;
unsigned char writePtr = 0;


WebSocketFrame wsFrame;
void ResetFlags(void) { flags.value = 0; }

void ReadWebSocket(void) {
    while (readPtr!=rxPtr) {
        ReadClient(rxBuffer[readPtr++]);
        readPtr%=MAXBYTES;
    } 
}
void WriteWebSocket(unsigned char tx) { 
    txBuffer[writePtr++]=tx;
    writePtr%=MAXBYTES;
}


void ReadClient(unsigned char rcv) {
    if (!flags.SOCKETCONNECT) {                     //lets see if there is a web socket identification key
        GetClientKeyIdent(rcv);
    } else {                                        //we're connected! here's the web socket frame.
        _LATB1 = 1;
        if (wsByteCount==1) payloadlen = rcv & 0x7f;
        if (wsByteCount<(payloadlen+5)) {           //watch for payload length, wait for the client to finish the write
            wsData[wsByteCount++]=rcv;
        } else {                                    //we've got all of it. now analyze the frame
            _LATB2 = 1;
            isConnected = 1;                        //debugging           
            wsData[wsByteCount++]=rcv;
            AnswerClient(wsData);
        }
    }
}

void GetClientKeyIdent(unsigned char rcv) {
    if (!flags.KEYFOUND) {
        if (rcv==WebSocketKeyIdent[keycntr]) keycntr++;                                         //lets see if we find a client web-socket key identification string
        if (keycntr==sizeof(WebSocketKeyIdent) && rcv!=WebSocketKeyIdent[keycntr]) keycntr=0;   //check if the key identification is still valid
        if (keycntr==sizeof(WebSocketKeyIdent)-1) {                                             //key identification found! read the next 24 chars for the key itself.
            flags.KEYFOUND = 1;
            keycntr = 0;
        } 
    } else {
        if (keycntr<WebSocketKeyLength) {                                                       //store the client key when the identification has been found
            WebSocketKey[keycntr] = rcv;
            if (keycntr==WebSocketKeyLength-1) {
                //we've got the key! now hash it and reply!
                //but no too fast! wait for the client to finish his write...
                //watch for CRLF CRLF, we have not switched protocols yet so HTTP still applies
                _LATB0 = 1;
            }
            keycntr++;
        }
        if (rcv==0x0d || rcv ==0x0a) {      //watch for CRLF CRLF
            cntrCRLF++;
            if (cntrCRLF==4) { 
                keycntr = 0;
                Handshake();       
            }
        } else if (cntrCRLF>0) cntrCRLF=0;
    }
}

void Handshake(void) {
    static HASH_SUM ReplyHash;
    char Sha1Result[20];
    char ResultBase64[40];
    unsigned char i;
    
    //hash the web socket key
    SHA1Initialize(&ReplyHash);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketKey, WebSocketKeyLength);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketGuid,(WORD)sizeof WebSocketGuid - 1);
    SHA1Calculate(&ReplyHash,(BYTE*)Sha1Result);
    
    //Base 64 encoding
    Base64encode(ResultBase64, Sha1Result, 20);
    
    //write out the reply.
    for (i=0;i<sizeof(ServerReply);i++) 
        WriteWebSocket(ServerReply[i]);
    
    //write out the hash
    
    writePtr--;                                         //disregard the null termination char of serverReply
    for (i=0;i<28;i++)                                  //28???? TODO: get the length from base64 encoding
        WriteWebSocket(ResultBase64[i]);
    
    //we still need to comply to HTTP protocol at this point so twice CRLF it is.
    WriteWebSocket(0x0d);
    WriteWebSocket(0x0a);
    WriteWebSocket(0x0d);
    WriteWebSocket(0x0a);

    flags.SOCKETCONNECT = 1;
    wsByteCount=0;
    cntrCRLF = 0;
    Commit();
}


void AnswerClient(unsigned char *msg) {
    unsigned char mask[4];
    char decoded[255];
    unsigned char i;
    
    wsFrame.value = msg[0];      
    
    switch (msg[0] & 0x0f) {    //opcode
        case 0x00:      //continuation frame
            break;
        case 0x01:      //text frame
            if (msg[1] & 0x80) {             
                payloadlen = msg[1] & 0x7f;     //TODO: check for payload length, limit this to minimize memory usage
                for (i=0;i<4;i++)               //read in the mask
                    mask[i] = msg[2+i];
                for (i=0;i<payloadlen;i++)      //decode the message
                    decoded[i] = msg[i+6]^mask[i%4];
                //at this point we have the complete message -> invoke command handler
                //echo the message back for now
                WriteWebSocket(0x81);           //FIN bit high and opcode=1
                WriteWebSocket(payloadlen);
                for (i=0;i<payloadlen;i++) {
                    WriteWebSocket(decoded[i]);
                }
            } else {
                //TODO: check for mask. frames from clients should be masked otherwise disconnect.
            }
            i=0;
            Commit();
            break;
        case 0x02:      //binary frame
            break;
        case 0x08:      //connection close
                        //if we didn't send a close frame first we should respond with a close frame
            WriteWebSocket(0x88);       //FIN bit high and opcode=8
            WriteWebSocket(0x00);       //we have no data
            flags.ISCONNECTED = 0;
            flags.KEYFOUND = 0;
            flags.SOCKETCONNECT = 0;
            LATB = 0;
            Commit();
            break;
        case 0x09:      //ping
            break;
        case 0x0a:      //pong
            break;
        default:        //reserved
            break;
    }
    wsByteCount = 0;                      //set the web socket read pointer to zero
    payloadlen = 0;
    
}
