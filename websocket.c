#include <p30f4013.h>
#include "websocket.h"
#include "sha1.h"
#include "base64.h"


//extern volatile unsigned char i2c_reg_map[];
//extern volatile unsigned int i2c_reg_addr;

extern unsigned char resetDevice;
extern unsigned char isConnected;               //debugging
extern unsigned char txUART[];
extern unsigned char txCount;

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


WebSocketFrame wsFrame;
void ResetFlags(void) { flags.value = 0; }

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
        if (keycntr==sizeof(WebSocketKeyIdent)-1) {                                                 //key identification found! read the next 24 chars for the key itself.
            flags.KEYFOUND = 1;
            keycntr = 0;
        } 
    } else {
        if (keycntr<WebSocketKeyLength) {                                                           //store the client key when the identification has been found
            WebSocketKey[keycntr] = rcv;
            if (keycntr==WebSocketKeyLength-1) {
                //we've got the key! now hash it and reply!
                //but no too fast! wait for the client to finish his write...
                //watch for CRLF CRLF, we have not switched protocols yet so HTTP still applies
                _LATB0 = 1;
            }
            keycntr++;
        }
        //watch for CRLF CRLF
        if (rcv==0x0d || rcv ==0x0a) { 
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
    //unsigned char txCount=0;
    
    //hash the web socket key
    SHA1Initialize(&ReplyHash);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketKey, WebSocketKeyLength);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketGuid,(WORD)sizeof WebSocketGuid - 1);
    SHA1Calculate(&ReplyHash,(BYTE*)Sha1Result);
    
    //Base 64 encoding
    Base64encode(ResultBase64, Sha1Result, 20);
    
    txCount = 0;            //we're gonna write. set the pointer to zero.
    
    //write out the reply, we're only transmitting from the main loop so place it in a buffer
    for (i=0;i<sizeof(ServerReply);i++) 
        txUART[txCount++] = ServerReply[i];
    
    //write out the hash
    
    txCount--;                      //disregard the null termination char of serverReply
    for (i=0;i<28;i++)                                  //28???? TODO: get the length from base64 encoding
        txUART[txCount++] = ResultBase64[i];
    
    //we need to comply to HTTP protocol at this point so twice CRLF it is.
    txUART[txCount++] = 0x0d;
    txUART[txCount++] = 0x0a;
    txUART[txCount++] = 0x0d;
    txUART[txCount++] = 0x0a;
    
    
    /*i2c_reg_addr=0;                             //we're gonna write, so set the pointer to zero
    for (i=0;i<sizeof(ServerReply);i++) 
        i2c_reg_map[i2c_reg_addr++] = ServerReply[i];
    
    //write out the hash
    i2c_reg_addr--;                                     //why is this needed??? (null value)
    for (i=0;i<28;i++)                                  //28???? TODO: get the length from base64 encoding
        i2c_reg_map[i2c_reg_addr++] = ResultBase64[i];
   
    //we need to comply to HTTP protocol at this point so twice CRLF it is.
    i2c_reg_map[i2c_reg_addr++] = 0x0d;
    i2c_reg_map[i2c_reg_addr++] = 0x0a;
    i2c_reg_map[i2c_reg_addr++] = 0x0d;
    i2c_reg_map[i2c_reg_addr++] = 0x0a;
    */
    flags.SOCKETCONNECT = 1;
    
    wsByteCount=0;
    cntrCRLF = 0;
}


void AnswerClient(unsigned char *msg) {
    unsigned char mask[4];
    char decoded[255];
    unsigned char i;
    
    wsFrame.value = msg[0];      
    //i2c_reg_addr=0;                 //we're gonna write, so set the pointer to zero
    
    //while (txUART[0]!=0);
    //txCount = 0;
    
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
                txCount = 0;                    //we're gonna write, so set the pointer to zero
                txUART[txCount++] = 0x81;       //FIN bit high and opcode=1
                txUART[txCount++] = payloadlen;
                for (i=0;i<payloadlen;i++) {
                    txUART[txCount++] = decoded[i];
                }
            } else {
                //TODO: check for mask. frames from clients should be masked otherwise disconnect.
            }
            break;
        case 0x02:      //binary frame
            break;
        case 0x08:      //connection close
                        //if we didn't send a close frame first we should respond with a close frame
            txCount = 0;                    //we're gonna write, so set the pointer to zero
            txUART[txCount++] = 0x88;       //FIN bit high and opcode=8
            txUART[txCount++] = 0x00;       //we have no data
            //txUART[txCount++] = 0x00;       //NULL
            flags.ISCONNECTED = 0;
            flags.KEYFOUND = 0;
            flags.SOCKETCONNECT = 0;
            LATB = 0;
            
            //resetDevice = 1;
            //i2c_reg_map[i2c_reg_addr++] = 0x00;
            
            //asm("RESET");   //total reset
            resetDevice = 1;    //total reset
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
