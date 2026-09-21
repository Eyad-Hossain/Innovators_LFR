
void line_follow() {

  reading();

  if (sum == 13) {  // All black

    if (inverse_flag && millis() - inverse_time > 25) inverse_flag = 0;

    if (!end_flag) {
      end_time = millis();
      end_flag = true;
    } else if (millis() - end_time > 30) {
      if (!brake_flag) {
        motor(-250, -250);
        delay(30);
        brake_flag = true;
      }
      motor(0, 0);
    }
  } else if (sum == 0) {  // All white

    if (inverse_flag && millis() - inverse_time > 25) inverse_flag = 0;

    if (break_flag && millis() - left_timer > 200 && millis() - right_timer > 200) {
      break_timer = millis();
      while (!sum) {
        motor(220, 220);
        reading();
        if (millis() - break_timer > 100) {

          // if (right_turn_detected) turn = 'l';
          // else if (left_turn_detected) turn = 'r';

          while (!sum) {

            if (right_timer > left_timer) motor(220, -220);
            else motor(-220, 220);
            reading();
          }
          break;
        }
      }

      hold_break_time = millis() - break_timer;
      hold_break_time1 += hold_break_time;
    }

    W += 2;
    if (!brake_flag) {
      brake_timer = millis();
      brake_flag = 1;
     // motor(-250, -250);
      if (millis() - straight_timer > 20) brake_threshold = 50;
      else brake_threshold = 10;
    } // else if (millis() - brake_timer > brake_threshold) motor(0, 0);



    if (right_timer > left_timer) {
      if (v_turn == 'l' && right_timer - left_timer < 300)
        motor(-220, 220);
      else motor(220, -220);
    } else {
      if (v_turn == 'r' && left_timer - right_timer < 300)
        motor(220, -220);
      else motor(-220, 220);
    }
  } else if (sum > 6 && (sensor & 0b0111000000000 && sensor & 0b0000000001110 && !(sensor & 0b0000000100000 && sensor & 0b0000001000000 && sensor & 0b0000010000000))) {
    if (!inverse_flag) {
      inverse_flag = 1;
      inverse_time = millis();
      motor(220, 220);
    } else if (millis() - inverse_time > 20) {
      i_mode = !i_mode;
      inverse_flag = 0;
      SL = 0;
      turn_SL = 0;
      SR = 0;
      turn_SR = 0;
      // turn_SR = 0;
      // turn_SL = 0;
      // SR = 0;
      // SL = 0;
    }
  } else {

    if (inverse_flag && millis() - inverse_time > 25) inverse_flag = 0;

    end_flag = 0;
    brake_flag = 0;

    //
    //    if (sum < 3 && millis() - turn_time > 20) {
    //      if (flag == 'l' && turn_detected == 'l') {
    //        while (sensor & 0b111100) {
    //          motor(-220, 220);
    //          reading();
    //        }
    //        while (!(sensor & 0b011100)) {
    //          motor(-220, 220);
    //          reading();
    //        }
    //
    //      }
    //      else if (flag == 'r' && turn_detected == 'r') {
    //        while (sensor & 0b001111) {
    //          motor(220, -220);
    //          reading();
    //        }
    //        while (!(sensor & 0b001110)) {
    //          motor(220, -220);
    //          reading();
    //        }
    //
    //      }
    //      turn_detected == '0';
    //    }


    if (millis() - turn_time > 50 && turn_flag && sum < 4 && (turn_SR || turn_SL) && (millis() - left_timer > 30 && millis() - right_timer > 30)) {

      if (turn == 'l' && turn_SL) {
        while (sensor & 0b1111111100000) {
          motor(-220, 220);
          reading();
        }
        while (!(sensor & 0b1111111000000)) {
          motor(-220, 220);
          reading();
        }
        // turn = 'r';

      } else if (turn == 'r' && turn_SR) {
        while (sensor & 0b0000011111111) {
          motor(220, -220);
          reading();
        }

        turn_count++;

        while (!(sensor & 0b0000001111111)) {
          motor(220, -220);
          reading();
        }

        // turn = 'l';

        if (turn_count > 3) {
          // turn_flag = 0;
          // break_flag = 1;
        }
      }
      turn_time = millis();
      SL = 0;
      turn_SL = 0;
      SR = 0;
      turn_SR = 0;
    }

    if (sensor & 0b0000001000000 || pos_flag) {

      if (pos != 0) {
        if (!pos_flag) {
          pos_flag = 1;
          pos_timer = millis();
          pos_time = abs(pos) * 10;
        }

        (pos > 0) ? motor(-250, 250) : motor(250, -250);
        // (pos > 0) ? delay(5 * pos) : delay(-5 * pos);

        if (millis() - pos_timer > pos_time) {
          pos = 0;
          pos_flag = 0;
        }
      }

      else
        motor(tsp, tsp);

      // if (!straight_flag) {
      //   //straight_timer = millis();
      //   straight_flag = 1;
      // }
    } else {
      //straight_timer = millis();
      // straight_flag = 0;

      if (sensor & 0b0000000100000) {
        motor(tsp, tsp);
        // if (pos < 1) pos = 1;
      } else if (sensor & 0b0000010000000) {
        motor(tsp, tsp);
        // if (pos < 1) pos = 1;
      } else if (sensor & 0b0000000010000) {
        motor(250, -250);
        if (pos < 1) pos = 1;
      } else if (sensor & 0b0000100000000) {
        motor(-250, 250);
        if (pos > -1) pos = -1;
      } else if (sensor & 0b0000000001111) {
        motor(250, -250);
        if (pos < 2) pos = 2;
      } else if (sensor & 0b1111000000000) {
        if (pos > -2) pos = -2;
        motor(-250, 250);
      }
    }
  }

  if (sum > 3) {

    //turn_complete = 1;
    // motor(150, 150);
    // straight_flag = 0;

    if (sensor & 0b0000000000011 || sensor & 0b0000000000101) {
      if (SR < 20) SR += 2;
    }

    if (sensor & 0b1100000000000 || sensor & 0b1010000000000) {
      if (SL < 20) SL += 2;
    }
  }


  if (sensor & 0b0000000000111) {
    if (R < 500) R += 50;
    //    if (sum > 2) {
    //      turn_detected = 'r';
    //      turn_time = millis();
    //      R = 500;
    //    }
    if (R > 400) right_timer = millis();
  }

  if (sensor & 0b1110000000000) {
    if (L < 500) L += 50;
    ///
    if (L > 400) left_timer = millis();
  }

  //  if ((sensor & 0b000001) && (sensor & 0b100000)) {
  //    if (turn == 'l') L = 500;
  //    else if (turn == 'r') R = 500;
  //  }

  if (SR > 10) {
    if (!turn_SR) {
      turn_SR = 1;
      right_timer = millis();
    }
  }

  if (SL > 10) {
    if (!turn_SL) {
      turn_SL = 1;
      left_timer = millis();
    }
  }

  if (left_timer > right_timer && left_timer - right_timer < 350) {
    turn_SL = 0;
    turn_SR = 0;
    SL = 0;
    SR = 0;
  } else if (left_timer < right_timer && right_timer - left_timer < 350) {
    turn_SL = 0;
    turn_SR = 0;
    SL = 0;
    SR = 0;
  }

  // if (turnSR) {
  //   right_turn_detected = 1;
  //   left_turn_detected = 0;
  // }


  // if (turnSL) {
  //   right_turn_detected = 0;
  //   left_turn_detected = 1;
  // }

  // if (i_mode && !counting_on) {
  //   inverse_time_counter = millis();
  //   counting_on = 1;
  // } else if (!i_mode) {
  //   counting_on = 0;
  //   inverse_time_counter = millis() - inverse_time_counter;
  // }

  // if (!counting_on && inverse_time_counter > 1000) {
  //   inverse_over = millis() + 2000;
  // }
  // if (inverse_over - millis() == 0) {
  //   turn_flag = 1;
  //   inverse_time_counter = 0;
  //   turn = 'r';
  // }

  if (sum && sum < 3 && (sensor & 0b1110000000111) == 0) {
    if (break_count < 580) break_count += 2;
  } else if (break_count > 5) break_count -= 5;

  if (break_count > 550) break_flag = 1;
  else break_flag = 0;
  // if(hold_break_time1 > 300) turn_flag = 0;

  if (sensor & 0b0000011100000) {
    if (!straight_flag) {
      straight_timer = millis();
      straight_flag = 1;
    } else if (millis() - straight_timer > 200) {
      if (tsp)
        brake_timer1 = millis();
      // tsp = 0;
    }
  } else {
    straight_flag = 0;
    straight_timer = millis();
    tsp = 220;
  }


  if (!tsp && millis() - brake_timer1 > 120) {
    tsp = 220;
    straight_timer = millis();
    straight_flag = 0;
  }
}

