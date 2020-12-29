const int DIN_PIN1 = 6;  // 正転用
const int DIN_PIN2 = 9;  // 逆転用
const int DIN_PIN3 = 12; // stop用
const int DIN_PIN4 = 11; // Auto用
const int MOTOR_PIN1 = 2;
const int MOTOR_PIN2 = 3;
const int PWM_PIN = 5;

const int sampling = 100;   // 100msごとにデータを取得する
const int long_boader = 30;  // 100 x 30 = 3秒間
const int auto_time[] = {1000,500,1000,3000}; //正転時間、待ち時間、逆転時間、待ち時間
const int pwm_forward = 100; // 分解能 1/256 
const int pwm_reverse = 200;

int btn_long_click[4] = {0,0,0,0};
int btn_stat[4] = {0,0,0,0};
unsigned long sw_time[] = {0,0,0,0};
int auto_state = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(DIN_PIN1,  INPUT_PULLUP);// デフォルトhigh
  pinMode(DIN_PIN2,  INPUT_PULLUP);
  pinMode(DIN_PIN3,  INPUT_PULLUP);
  pinMode(DIN_PIN4,  INPUT_PULLUP);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
}

//----- Start Device ----------------------------------//

// モーター正転
void motor_forward()
{
    digitalWrite(MOTOR_PIN1 ,HIGH);
    digitalWrite(MOTOR_PIN2 ,LOW);
    analogWrite(PWM_PIN,pwm_forward);
}

// モーター逆転
void motor_reverse()
{
    digitalWrite(MOTOR_PIN2 ,HIGH);
    digitalWrite(MOTOR_PIN1 ,LOW);
    analogWrite(PWM_PIN,pwm_reverse);
}

// モーター停止
void motor_stop()
{
    digitalWrite(MOTOR_PIN1 ,LOW);
    digitalWrite(MOTOR_PIN2 ,LOW);
}

/*------------------------
 state で管理する。
auto_state = 0 → 自動回転停止
auto_state = 1 → 自動正転
auto_state = 2 → 回転停止(待ち)
auto_state = 3 → 自動逆転
auto_state = 4 → 回転停止 → 1へ
-------------------------*/
void motor_auto()
{
    if(auto_state == 0 )
    {
        sw_time[0] = millis();
        auto_state = 1;
    }
    else if(auto_state == 1)
    {
        motor_forward();

        if(millis() - sw_time[0] > auto_time[0])
        {
            auto_state = 2;
            sw_time[0] = millis();
        }
    }
    else if(auto_state == 2)
    {
        motor_stop();
        if(millis() - sw_time[0] > auto_time[1])
        {
            auto_state = 3;
            sw_time[0] = millis();
        }
    }
    else if(auto_state == 3)
    {
        motor_reverse();
        if(millis() - sw_time[0] > auto_time[2])
        {
            auto_state = 4;
            sw_time[0] = millis();
        }
    }
    else if(auto_state == 4)
    {
        motor_stop();
        if(millis() - sw_time[0] > auto_time[3])
        {
            auto_state = 1;
            sw_time[0] = millis();
        }
    }
}

//----- Start Func Logic -----------------------------//
void motor_roll()
{
    // ボタン1が有効な時は、正転する。
    if(btn_stat[1] == 1)
    {
        motor_forward();
    }
    
    // ボタン1またはボタン2が押されていない。両方のボタンの長時間押し無効または、autoではないとき止める
    if( (btn_stat[1] == 0 && btn_stat[2] == 0) && (btn_long_click[1] == 0 && btn_long_click[2] == 0) && btn_stat[0] == 0 )
    {
        motor_stop();
    }

    // ボタン2が有効な時は、逆転する。
    if(btn_stat[2] == 1)
    {
        motor_reverse();
    }
    
    if(btn_stat[3] == 1)
    {
         motor_stop();
    }

    if(btn_stat[0] == 1)
    {
        
        motor_auto();
    }
}

// forward , reverse button
void btn_click(int btn_pin,int btn_no)
{
    // autoが機能していない時のみ手動で操作できる
    if(btn_stat[0] == 0)
    {
        // ボタンを初めて押した
        if(digitalRead(btn_pin) == LOW && btn_stat[btn_no] != 1)
        {
            btn_long_click[btn_no] = 0;
            btn_stat[btn_no] = 1;
            sw_time[btn_no] = millis();

        }else if(digitalRead(btn_pin) == LOW && btn_stat[btn_no] == 1)
        {
            // ボタンを4秒以上押した
            if((millis() - sw_time[btn_no]) > (sampling * long_boader))
            {
                btn_long_click[btn_no] = 1;
            }
        }
        else
        {
            btn_stat[btn_no] = 0;     
            sw_time[btn_no] = 0;
        }
    }
}

// stop button
void btn_click2(int btn_pin,int btn_no)
{
    if(digitalRead(btn_pin) == LOW)
    {
        btn_stat[btn_no] = 1;
        btn_stat[0] = 0; // autoを停止する
        btn_long_click[1] = 0;
        btn_long_click[2] = 0;
    }
    else
    {
        btn_stat[btn_no] = 0;        
    }
}

void btn_auto(int btn_pin,int btn_no)
{
    if(digitalRead(btn_pin) == LOW)
    {
        btn_stat[btn_no] = 1;
        btn_long_click[1] = 0;
        btn_long_click[2] = 0;
    }
}
//----- End Func Logic -------------------------------//

void loop()
{
    btn_click(DIN_PIN1,1);
    btn_click(DIN_PIN2,2);
    btn_click2(DIN_PIN3,3);
    btn_auto(DIN_PIN4,0);

    motor_roll();
    delay(sampling);
}