void setup()
{
Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  disp_setup();
  ms.get_root_menu().add_item(&mm_mi1);
  ms.get_root_menu().add_item(&mm_mi2);
  ms.get_root_menu().add_menu(&mu1);
  mu1.add_item(&mu1_mi1);
}

void disp_setup(){
  display.setTextSize(2);
  display.setTextSize(.5);
  display.setTextColor(WHITE);
}

void loop()
{
  
  ms.display();

  // Simulate using the menu by walking over the entire structure.
  ms.select();
  ms.next();

  if (done)
  {
    ms.reset();
    done = false;
  }

  delay(2000);
}
