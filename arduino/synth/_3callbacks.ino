void button_1_pressed(){
  if (edit_mode){
    ms.back();
    ms.display();
  }
}

void button_1_released(){
  if (!edit_mode){
    if (!playing){
      change_octave(-1);
      disp_update_non_menu();
    }
  }
}

void button_2_pressed(){
  if (edit_mode){
    scrolling=true;
    select_on_release=true;
  }
}

void button_2_released(){
  if (edit_mode){
    scrolling=false;
    if (select_on_release){
      ms.select();
      ms.display();
    }
  }else{
    if (!playing){
      change_octave(1);
      disp_update_non_menu();
    }
  }
}


int oto_tune(int f_orig){
  int freq;
  freq = round(log(f_orig/a_tune)/log_a_exp);
  freq = a_tune * exp(freq * log_a_exp);
  return freq;
}

void read_scale_neck() {
  // resistance of neck, accounting for voltage division: PULL * (max_adc-N)/N
  pot_val = analogRead(A10);
  if (pot_val < thresh_adc) pot_val = 0;
  else {
    pot_val = (neckscale/pot_val-PULL-RMIN)*neckscale_2;
  } 
  avgs[avg_ind] = pot_val;
  avg_ind = (avg_ind + 1) % FILTER_SIZE; 
}
