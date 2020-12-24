

enum ParseState
  {
   PARSE_STATE_SEARCHING,
   PARSE_STATE_SIZE,
   PARSE_STATE_DATA
  };

typedef struct 
  {
  uint8_t       index;
  uint8_t       data_sz;
  ParseState    state;
  char          buf[80];
  uint8_t       bin[40];
  } parse_state_t;

typedef struct __attribute__((__packed__))
  {
   uint8_t data_sz;
   uint16_t serial;
   uint16_t tick;
   uint8_t type;
  } Header_t;

void parse(char c, parse_state_t * cb);

parse_state_t parser;

void setup() {
  // put your setup code here, to run once:

  Serial2.begin(38400);
  Serial.begin(115200);
  

memset(&parser, 0, sizeof(parser) );

 
}


// AT+INIT
// AT+INQ
// AT+PAIR=12,6F,63B884,5
// AT+BIND=12,6F,63B884
// AT+LINK=12,6F,63B884

// 12,6F,63B884

void loop() {
  // put your main code here, to run repeatedly:

 //from bluetooth to Terminal. 
 if (Serial2.available()) 
   parse( Serial2.read(), &parser );

/*
 //from termial to bluetooth 
 if (Serial.available()) 
   Serial2.write(Serial.read());
*/

/*

char * data = "TELEM 224510240804250100011780631513c0074e30574d50000000312e382e37000000528837";
parse_state_t parser;

memset(&parser, 0, sizeof(parser) );
for( int i = 0; i < strlen(data); i++ )
  {
  Serial.print("Handling: ");Serial.println(data[i]);
  parse( data[i], &parser );
  delay(250);
  }
*/

}




void parse(char c, parse_state_t * cb)
{

switch( cb->state )
  {
  case PARSE_STATE_SEARCHING:
    {
    if( c == ' ' )
      {
      cb->index = 0;
      memset( cb->buf, 0, sizeof(cb->buf) );
      cb->state = PARSE_STATE_SIZE;
      break;
      }
    break;
    }
  case PARSE_STATE_SIZE:
    {
    cb->buf[cb->index++] = c;

    if( cb->index >= 2 )
      {
      cb->state = PARSE_STATE_DATA;
      hextobin( cb->buf, &(cb->data_sz), 1 );
      // We want this to be strlen, not binary len
      cb->data_sz *= 2;
      //Serial.print("Size ");Serial.println(cb->data_sz);
      }
    break;
    }
   case PARSE_STATE_DATA:
    {
    cb->buf[cb->index++] = c;

    if( cb->index >= 2 + 2 + cb->data_sz ) // 2 bytes size, 2 bytes crc
      {
      cb->buf[cb->index]='\0';
      //Serial.println("Finished");
      Serial.println(cb->buf);

      hextobin(cb->buf, cb->bin, ( cb->data_sz + 2 + 2 ) / 2 );

      Header_t * header = (Header_t *)cb->bin;

      Serial.print("Binary Size: ");Serial.println(header->data_sz);
      Serial.print("Serial: ");Serial.println(header->serial);
      Serial.print("Tick: ");Serial.println(header->tick);
      Serial.print("Type: ");Serial.println(header->type);
      Serial.println();

      memset(&parser, 0, sizeof(parser));
      }
      
    }

   
    break;
  
  }

  //Serial.print("State ");Serial.println(cb->state);
  
}


uint8_t hextobin(const char * str, uint8_t * bytes, size_t blen)
{
   uint8_t  pos;
   uint8_t  idx0;
   uint8_t  idx1;

   // mapping of ASCII characters to hex values
   const uint8_t hashmap[] =
   {
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
     0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
   };

   bzero(bytes, blen);
   for (pos = 0; ((pos < (blen*2)) && (pos < strlen(str))); pos += 2)
   {
      idx0 = (uint8_t)str[pos+0];
      idx1 = (uint8_t)str[pos+1];
      bytes[pos/2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
   };

   return(0);
}
