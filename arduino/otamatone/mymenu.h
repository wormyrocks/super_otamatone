#define MAX_NUM_PRESETS 10
#define cur_preset preset_no.get_value ()
#define BIG 5
#define UNDERLINE 4
#define HIGHLIGHT_TEXT 1
#define HIGHLIGHT_NONE 0
#define H_CHARS 22
#define INDENT (H_CHARS >> 1)
class MyRenderer: public MenuComponentRenderer
{
  public:
    virtual void render(Menu const & menu) const
    {
      menu.fs_render(* this);
    }
    virtual void fs_menu_render(Menu const & menu) const
    {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.setTextColor(WHITE);
      print_centered(menu.get_name(), UNDERLINE);
      oled.setCursor(0, 16 + 4 * (6 - menu.get_num_components()));
      for (int i = 0; i < menu.get_num_components(); ++i)
      {
        MenuComponent const * cp_m_comp = menu.get_menu_component(i);
        cp_m_comp -> render(* this);
        oled.println("");
      }
      oled.display();
    }
    virtual void fs_bignumber_render(BigNumberSlider const & big_number) const
    {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      print_centered(big_number.get_name(), UNDERLINE);
      uint8_t a, b, c, d;
      uint8_t anim = big_number.get_animation();
      if (anim < 10)
      {
        switch (anim)
        {
          case 1:
            c = big_number.get_value() * 0.5;
            d = 48;
            b = 16;
            a = 0;
            break;
          case 2:
            c = big_number.get_value() * 0.5;
            d = 48;
            b = 16;
            a =.5 * (128 - c);
            break;
          case 3:
            d = big_number.get_value() * 0.1875;
            b = 64 - d;
            c = 128;
            a = 0;
            break;
          case 4:
            d = big_number.get_value() * 0.1875;
            b = 40 - d/2;
            c = 128;
            a = 0;
            break;
          case 5:
            d = 0.1875 * big_number.get_value();
            c = d * 2.66;
            b = 16 + (48 - d) /2;
            a = (128 - c) /2;
            break;
        }
        oled.fillRect(a, b, c, d, WHITE);
      }
      else
        if (anim < 20)
        {
          switch (anim)
          {
            case 10:
              a =.09375 * big_number.get_value();
              for (uint8_t n = 0; n < a; n += 2)
              {
                oled.drawRect(n, 16 + n, 128 - n * 2, 48 - 2 * n, WHITE);
              };
              break;
            case 11:
              oled.fillCircle(64, 40,.09375 * big_number.get_value(), WHITE);
              break;
          }
        }
      oled.setCursor(oled.getCursorX(), 25);
      char buf0[3];
      itoa(big_number.get_value(), buf0, 10);
      // if (big_number.get_isnew()){
      // char buf1[5];
      // strcpy(buf1,"[");
      // strcat(buf1, buf0);
      // strcat(buf1,"]");
      // print_centered(buf1, BIG);
      // }else print_centered(buf0, BIG);
      print_centered(buf0, BIG);
      oled.display();
    }
    virtual void render_big_number_item(BigNumberSlider const & big_number) const
    {
      char buf[22];
      sprintf(buf, "%s:%d", big_number.get_name(), big_number.get_value());
      print_centered(buf, big_number.is_current()? HIGHLIGHT_TEXT:HIGHLIGHT_NONE);
    }
    virtual void render_menu_item(MenuItem const & menu_item) const
    {
      print_centered(menu_item.get_name(), menu_item.is_current()? HIGHLIGHT_TEXT:HIGHLIGHT_NONE);
    }
    virtual void render_numeric_menu_item(NumericMenuItem const & menu_item) const
    {
      int d2 = trunc(menu_item.get_value());
      if (menu_item.has_focus())
      {
        oled.clearDisplay();
        oled.setCursor(0, 0);
        oled.setTextSize(1);
        print_centered(menu_item.get_name(), UNDERLINE);
        oled.setCursor(0, 32);
        oled.setTextSize(2);
        oled.print(d2);
        oled.setTextSize(1);
      }
      else
      {
        char s[strlen(menu_item.get_name()) + 3];
        sprintf(s, "%s %d", menu_item.get_name(), d2);
        print_centered(s, menu_item.is_current()? HIGHLIGHT_TEXT:HIGHLIGHT_NONE);
      }
    }
    virtual void render_back_menu_item(BackMenuItem const & menu_item) const
    {
      oled.print(menu_item.get_name());
    }
    virtual void render_menu(Menu const & menu) const
    {
      char s[strlen(menu.get_name()) + 2];
      strcpy(s, menu.get_name());
      strcat(s, "..");
      print_centered(s, menu.is_current()? HIGHLIGHT_TEXT:HIGHLIGHT_NONE);
    }
  private:
    void print_centered(const char * str, uint8_t highlight) const
    {
      uint8_t c_size = 0;
      uint8_t buf_size = strlen(str);
      if (highlight == BIG)
      {
        c_size = 2;
        oled.setTextColor(INVERSE);
      }
      if (highlight == HIGHLIGHT_TEXT)
      {
        buf_size = buf_size + 2;
        if (buf_size > (H_CHARS >> c_size))
        {
          buf_size = (H_CHARS >> c_size);
        }
      }
      uint8_t x_start = (128 - buf_size * (6 << c_size)) >> 1;
      oled.setTextSize(1 << c_size);
      oled.setCursor(x_start, oled.getCursorY());
      if (highlight == HIGHLIGHT_TEXT)
      {
        oled.setTextColor(BLACK, WHITE);
        oled.print(" ");
      }
      oled.print(str);
      if (highlight == HIGHLIGHT_TEXT)
        oled.print(" ");
      oled.setTextColor(WHITE);
      if (highlight == UNDERLINE)
      {
        oled.setCursor(x_start, oled.getCursorY() + (8 << c_size));
        uint8_t i = 0;
        uint8_t j = strlen(str);
        while (i < j)
        {
          oled.print("-");
          i += 1;
        }
        oled.println("");
      }
      oled.setTextSize(1);
    };
};

MyRenderer my_renderer;
MenuSystem ms(my_renderer);
void update_bignumber_preset(Menu * p_menu);
void update_preset_menu();
void preset_en(MenuItem * p_menu_item);
Menu edit_preset("Edit Preset");
BigNumberSlider preset_no("Editing Preset", 0, 0, MAX_NUM_PRESETS - 1, 1, 1, & update_bignumber_preset);
MenuItem enable_preset("Disable Preset", & preset_en);
Menu env_edit("Edit Chain");
Menu eff_edit("Edit Effects");
// MenuItem eff_0("New Effect", NULL);
// Menu eff_1("Effect 1");
// Menu eff_2("Effect 2");
// Menu eff_3("Effect 3");
// Menu eff_4("Effect 4");
// 
// Menu trigger_type("Trigger On");
// MenuItem btn1_0("Always On");
// MenuItem btn1_1("Button 1 Press");
// MenuItem btn1_2("Button 1 Double Press");
// MenuItem btn1_3("Button 1 Hold");
// MenuItem btn2_1("Button 2 Press");
// MenuItem btn2_2("Button 2 Double Press");
// MenuItem btn2_3("Button 2 Hold");
// Menu effect_type("Effect Type");
// MenuItem eff1("Oto Tune");
// MenuItem eff2("Chorus");
// MenuItem eff3("Delay");
// MenuItem eff4("Bit Crusher");
// MenuItem eff5("Flanger");
// MenuItem eff6("Reverb");
// MenuItem eff7("LED Effects");
Menu main_settings("Settings");
typedef struct
{
  bool en;
  uint8_t btn1_effect_type;
  uint8_t btn1_effect;
  uint8_t btn2_effect_type;
  uint8_t btn2_effect;
} preset;
preset presets[MAX_NUM_PRESETS];
bool preset_menu_shown;
void update_bignumber_preset(Menu * p_menu)
{
  update_preset_menu();
}

void update_preset_menu()
{
  if (presets[cur_preset].en != preset_menu_shown)
  {
    preset_menu_shown = presets[cur_preset].en;
    preset_menu_shown? edit_preset.show_n_items(2):edit_preset.hide_n_items(2);
  }
  // preset_no.set_isnew(presets[cur_preset].en);
  enable_preset.set_name(preset_menu_shown? "Disable Preset":"Enable Preset");
}

void preset_en(MenuItem * p_menu_item)
{
  presets[cur_preset].en = !presets[cur_preset].en;
  update_preset_menu();
}

void disp_setup(){
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
}

void disp_update_non_menu(){
  int octave = synth1.get_octave();
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.print("Octave: ");
  if (octave >= 0) oled.print("+");
  oled.println(octave);
  oled.display();
}

void populate_menus(){
  Menu &root = ms.get_root_menu();
  root.set_name("Patch Editor");
  root.add_menu(&edit_preset);
  root.add_menu(&main_settings);
  edit_preset.add_menu(&preset_no);
  edit_preset.add_item(&enable_preset);
  edit_preset.add_menu(&env_edit);
  edit_preset.add_menu(&eff_edit);
}

void update_menu()
{
  if (scrolling){
    if (pot_val != 0){
      Menu const* m = ms.get_current_menu();
      int index = (max_adc-pot_val) * m->get_num_components() / max_adc;
      if (index > m->get_current_component_num()){
        ms.next();
        ms.display();
      }else if (index < m->get_current_component_num()){
        ms.prev();
        ms.display();
      }
      select_on_release = false;
    }
  }
}

