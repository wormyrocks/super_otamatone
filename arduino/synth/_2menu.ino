Adafruit_SSD1306 display(OLED_RESET);

class MyRenderer : public MenuComponentRenderer
{
public:
    virtual void render(Menu const& menu) const
    {
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("");
        for (int i = 0; i < menu.get_num_components(); ++i)
        {
            MenuComponent const* cp_m_comp = menu.get_menu_component(i);
            
            if (cp_m_comp->is_current())
                display.setTextColor(BLACK, WHITE);
            else display.setTextColor(WHITE);
            
            cp_m_comp->render(*this);
           
            display.println("");
        }
        display.display();
    }

    virtual void render_menu_item(MenuItem const& menu_item) const
    {
        display.print(menu_item.get_name());
    }

    virtual void render_back_menu_item(BackMenuItem const& menu_item) const
    {
        display.print(menu_item.get_name());
    }

    virtual void render_numeric_menu_item(NumericMenuItem const& menu_item) const
    {
        display.print(menu_item.get_name());
    }

    virtual void render_menu(Menu const& menu) const
    {
        display.print(menu.get_name());
    }
};

MyRenderer my_renderer;

MenuSystem ms(my_renderer);

void on_item1_selected(MenuItem* p_menu_item);
void on_item2_selected(MenuItem* p_menu_item);
void on_item3_selected(MenuItem* p_menu_item);
void on_item4_selected(MenuItem* p_menu_item);

MenuItem mm_mi1("1", &on_item1_selected);
MenuItem mm_mi2("2", &on_item2_selected);
Menu mu1("3");
MenuItem mu1_mi1("Oto Tune Disabled", &on_item3_selected);
MenuItem mu1_mi2("4", &on_item4_selected);

void on_item1_selected(MenuItem* p_menu_item)
{
}

void on_item2_selected(MenuItem* p_menu_item)
{
}

void on_item3_selected(MenuItem* p_menu_item)
{
  oto_tune_on = !oto_tune_on;
  if (oto_tune_on){
    p_menu_item->set_name("Oto Tune Enabled");
  }else p_menu_item->set_name("Oto Tune Disabled");
}
void on_item4_selected(MenuItem* p_menu_item){
  
}



