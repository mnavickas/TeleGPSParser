

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


typedef struct __attribute__((__packed__))
  {
  uint8_t flags;
  int16_t altitude;
  int32_t latitude;
  int32_t longitude;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t pdop;
  uint8_t hdop;
  uint8_t vdop;
  uint8_t mode;
  uint16_t ground_speed;
  int16_t climb_rate;
  uint8_t course;
  uint8_t unused[1];
  } gps_location_data_t;


void parse(char c, parse_state_t * cb);

parse_state_t parser;

void setup() {
  // put your setup code here, to run once:

  Serial2.begin(38400);
  Serial.begin(38400);
  
  delay(1000);
  Serial2.print("AT+INIT\r\n");
  delay(1000);
  Serial2.print("AT+INQ\r\n");
  delay(5000);
  Serial2.print("AT+PAIR=12,6F,63B884,5\r\n");
  delay(2000);
  Serial2.print("AT+BIND=12,6F,63B884\r\n");
  delay(2000);
  Serial2.print("AT+LINK=12,6F,63B884\r\n");
  delay(5000);

  while( Serial2.available() )
  {
    Serial.write(Serial2.read());
  }
  Serial2.print("E 0\nm 0\n");
  Serial2.print("c F 435650\n");
  Serial2.print("c T 0\n");

  while( Serial2.available() )
  {
    Serial.write(Serial2.read());
  }
  Serial2.print("m 20\n");
  delay(500);

  Serial.println("Init Done");
  

memset(&parser, 0, sizeof(parser) );

 
}


// AT needs \r\n
// ALTOS needs \n

// AT+INIT
// AT+INQ
// AT+PAIR=12,6F,63B884,5
// AT+BIND=12,6F,63B884
// AT+LINK=12,6F,63B884

// 12,6F,63B884

void loop() {
  // put your main code here, to run repeatedly:

 //from bluetooth to Terminal. 

 if( Serial2.available() ) 
   parse( Serial2.read(), &parser );
 
}


void handleGPSLocationData(uint8_t * buf)
  {
  gps_location_data_t * gpsdata = (gps_location_data_t*)buf;
  
  Serial.print("Soln. Valid ");Serial.println((bool) (gpsdata->flags & (1<<4)) );
  Serial.print("Altitude ");Serial.println(gpsdata->altitude);
  Serial.print("Latitude ");Serial.println(gpsdata->latitude / 10000000.0f, 6);
  Serial.print("Longitude ");Serial.println(gpsdata->longitude / 10000000.0f, 6);
  Serial.print("Minute ");Serial.println(gpsdata->minute);
  Serial.print("Second ");Serial.println(gpsdata->second);
  
  Serial.print("PDOP ");Serial.println(gpsdata->pdop / 5.0, 4);
  Serial.print("HDOP ");Serial.println(gpsdata->hdop / 5.0, 4);
  Serial.print("VDOP ");Serial.println(gpsdata->vdop / 5.0, 4);
  Serial.println();
  }
void handleCompleted(parse_state_t * cb)
{
//Serial.println("Finished");
Serial.println(cb->buf);

hextobin(cb->buf, cb->bin, ( cb->data_sz + 2 + 2 ) / 2 );

uint8_t * actualcrc = &(cb->bin[35]);
uint8_t computedcrc = 0x5A;
for( int i =1; i< 35; i ++ )
  {
  // over:flow intentional
  computedcrc += cb->bin[i];
  }
Serial.print("CRC: ");Serial.print(*actualcrc); Serial.print(" Computed: ");Serial.println(computedcrc); 


if( *actualcrc == computedcrc )
  {
  Header_t * header = (Header_t *)cb->bin;
  
  Serial.print("Binary Size: ");Serial.println(header->data_sz);
  Serial.print("Serial: ");Serial.println(header->serial);
  Serial.print("Tick: ");Serial.println(header->tick);
  Serial.print("Type: ");Serial.println(header->type);

  switch(header->type)
    {
    case 0x05:
      handleGPSLocationData( &cb->bin[6] );
    }
  
  }




memset(&parser, 0, sizeof(parser));
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

      handleCompleted( cb );

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
