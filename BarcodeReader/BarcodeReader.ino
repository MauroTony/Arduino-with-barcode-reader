#include <usbhid.h>
#include <usbhub.h>
#include <hiduniversal.h>
#include <hidboot.h>
#include <SPI.h>
#include <UIPEthernet.h> 

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 27 };
byte gt[] = { 192, 168, 0, 1 }; 
byte webserver[] = { 192, 168, 0, 8 }; 
/*
byte ip[] = { 10, 0, 0, 27 };
byte gt[] = { 10, 0, 0, 254 }; 
byte webserver[] = { 190, 15, 112, 93 }; 
 */
EthernetServer server(80);

String cd_dispisitivo = "367";
#define RELAY1 31
#define RELAY2 33
#define RELAY3 35
#define RELAY4 37

String codigo = "";
String v_request = "";

int leitura = 0;

class MyParser : public HIDReportParser {
  public:
    MyParser();
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
  protected:
    uint8_t KeyToAscii(bool upper, uint8_t mod, uint8_t key);
    virtual void OnKeyScanned(bool upper, uint8_t mod, uint8_t key);
    virtual void OnScanFinished();
};

MyParser::MyParser() {}

void MyParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  // If error or empty, return
  if (buf[2] == 1 || buf[2] == 0) return;

  for (uint8_t i = 7; i >= 2; i--) {
    // If empty, skip
    if (buf[i] == 0) continue;

    // If enter signal emitted, scan finished
    if (buf[i] == UHS_HID_BOOT_KEY_ENTER) {
      leitura = 0;
      OnScanFinished();
    }

    // If not, continue normally
    else {
      // If bit position not in 2, it's uppercase words
            leitura = 1;
      OnKeyScanned(i > 2, buf, buf[i]);

    }

    return;
  }
}

uint8_t MyParser::KeyToAscii(bool upper, uint8_t mod, uint8_t key) {
  // Letters
  if (VALUE_WITHIN(key, 0x04, 0x1d)) {
    if (upper) return (key - 4 + 'A');
    else return (key - 4 + 'a');
  }

  // Numbers
  else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
    return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
  }

  return 0;
}

void MyParser::OnKeyScanned(bool upper, uint8_t mod, uint8_t key) {
  uint8_t ascii = KeyToAscii(upper, mod, key);
  //Serial.print((char)ascii);
  codigo = codigo + (char)ascii;
}

void MyParser::OnScanFinished() {
  Serial.println(codigo);
  EthernetClient client = server.available();
  if (client.connect(webserver, 80)) {
    Serial.println("connected");
    v_request = "GET /Qrcode_Arduino/index.php?codigo=" + codigo + "&cd_dispisitivo=" + cd_dispisitivo + " HTTP/1.1";
    Serial.println(v_request);
    client.println(v_request);
    client.println("Host: 192.168.0.8");
    client.println("Connection: close");
    client.println();
  }
  else {
  Serial.println("connection failed");
  }
  client.stop();    
  Serial.print(codigo);
  codigo = "";
  Serial.println(" - Finished");
}

USB          Usb;
USBHub       Hub(&Usb);
HIDUniversal Hid(&Usb);
MyParser     Parser;

void setup() {
  Serial.begin( 115200 );
  Serial.println("Start");
  
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }

  delay( 200 );

  Ethernet.begin(mac, ip, gt, gt);
  server.begin();

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, LOW);          // Turns ON Relays 1
  delay(350);                         // Wait 0,1 seconds
  digitalWrite(RELAY1, HIGH);
  delay(350);                         // Wait 0,1 seconds
  digitalWrite(RELAY2, LOW);          // Turns ON Relays 1
  delay(350);                         // Wait 0,1 seconds
  digitalWrite(RELAY2, HIGH);
  delay(350); 
  digitalWrite(RELAY3, LOW);          // Turns ON Relays 1
  delay(350);                         // Wait 0,1 seconds
  digitalWrite(RELAY3, HIGH);
  delay(350); 
  digitalWrite(RELAY4, LOW);          // Turns ON Relays 1
  delay(350);                         // Wait 0,1 seconds
  digitalWrite(RELAY4, HIGH);
  delay(350); 
    
  Hid.SetReportParser(0, &Parser);
}

void loop() {
  if(leitura == 0){
      Usb.Task();
  }
  if(leitura == 1){
    while(leitura == 1){
       Usb.Task();
    }
  }

  
  EthernetClient client = server.available();
  String request = client.readStringUntil('\r');
  client.flush();
  Serial.println(client.available());
  if (request.indexOf("/RELE1") != -1) {
    //client.print("------ RELE 1 ACIONADO-----");
    digitalWrite(RELAY1, LOW);         
    delay(500);                       
    digitalWrite(RELAY1, HIGH);
    delay(500);               
    client.stop();       
  }
  else if (request.indexOf("/RELE2") != -1) {
    //client.print("------ RELE 2 ACIONADO-----");
    digitalWrite(RELAY2, LOW);         
    delay(500);                        
    digitalWrite(RELAY2, HIGH);
    delay(500);
    client.stop();    
  }
  else if (request.indexOf("/RELE3") != -1) {
    //client.print("------ RELE 3 ACIONADO-----");
    digitalWrite(RELAY3, LOW);          
    delay(500);                         
    digitalWrite(RELAY3, HIGH);
    delay(500);
    client.stop();                       
  }
  else if (request.indexOf("/RELE4") != -1) {
    //client.print("------ RELE 4 ACIONADO-----");
    digitalWrite(RELAY4, LOW);       
    delay(500);                        
    digitalWrite(RELAY4, HIGH);
    delay(500);
    client.stop();                  
  }
  else {
    client.stop();
  }
  
}
