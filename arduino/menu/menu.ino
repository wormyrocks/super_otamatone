#include <MenuSystem.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4

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
            cp_m_comp->render(*this);

            if (cp_m_comp->is_current())
                display.print("<<< ");
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

void on_item1_selected(MenuItem* p_menu_item);
void on_item2_selected(MenuItem* p_menu_item);
void on_item3_selected(MenuItem* p_menu_item);

// Menu variables

MenuSystem ms(my_renderer);

MenuItem mm_mi1("Level 1 - Item 1 (Item)", &on_item1_selected);
MenuItem mm_mi2("Level 1 - Item 2 (Item)", &on_item2_selected);
Menu mu1("Level 1 - Item 3 (Menu)");
MenuItem mu1_mi1("Level 2 - Item 1 (Item)", &on_item3_selected);

// Menu callback function

bool done = false;

void on_item1_selected(MenuItem* p_menu_item)
{
  Serial.println("Item1 Selected");
}

void on_item2_selected(MenuItem* p_menu_item)
{
  Serial.println("Item2 Selected");
}

void on_item3_selected(MenuItem* p_menu_item)
{
  Serial.println("Item3 Selected");
  done = true;
}


