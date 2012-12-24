/*
 * Simple traffic light controller for arduino
 * 
 * I have written this sketch to control 4 traffic lights in a lego city
 * the small round leds fit perfectly in lego's small armatures used in
 * their small cars for front and back lights.
 * 
 * I stackt 3 for each traffic light with  a red, yellow and green light.
 * The twelve leds are configured in a charlieplexed mode as follows:
 * 
 * For two of the lights:                For the other two:
 *   
 *   +-->|---+           green              +---|<---+
 *   +--|<-+ |           yellow             |  +--|<-+
 *   +->|--+ |           red                |  +->|--+
 *   |     | |                              |  |     |
 * A 0     1 2   pins on the controller   B 0  1     2
 * C 2     3 4                              2  3     4
 * 
 * all pins are connected to the microcontroller using 220 Ohm when running at 5V
 * 
 * I thought that i had one pin on the controller free for a button
 * But iturns out that the pin is the reset pin.
 * 
 * So i used pin 1 as button pin.
 * As this pin is both used to control 4 leds (in conjunction with other pins ofcourse)
 * and to read a button press.
 * I switch the pin to +5V  over a 10K resistor so the current won't be enough to 
 * light a led. And I pull the pin to ground with a 22K resistor in series with the 10K resistor.
 * 
 *
 *          +5v o 
 * pin 1        /- (switch)
 *     o--[10K]-+
 *              |
 *              +-[22K]-o ground 
 * 
 * Later on I realized that I could have used just 4 pins to charlieplex the leds.
 * And this means that I would have a completely free pin for input.
 * 
 * The code below still uses 5 pins for the leds and a shared pin for the button.
 */

#define MAXLIGHTS 4
#define PINOFFSET 0
#define GREEN_TIMEOUT 30000  // The time (in ms) to light the green led
#define ORANGE_TIMEOUT 3500 // The time (in ms) to light the orange led
#define RED_TIMEOUT 1200 // The time (in ms) to light all leds red before turning on the next green led
#define BUTTON_PIN 1

struct traffic_light_s;

struct control_s
{
  int button_pressed;
  int button_value;
  struct traffic_light_s * lights;
  unsigned long timeout;
  int current;
};


void control_next(struct control_s * control)
{
  control->current++;
  if(control->current >= MAXLIGHTS)
    control->current = 0;
}

enum colors {
  RED,
  ORANGE,
  GREEN
};

struct led_s
{
  int a;
  int c;
};
/*
 * Because I might want to use pin 0 and 1 for serial
 * but I still want to number the charlieplexed leds 0 to 5
 * we can configure the PINOFFSET to 2
 */
void init_led(struct led_s * led, int anode, int cathode)
{
  led->a = anode + PINOFFSET;
  led->c = cathode + PINOFFSET; 
}

struct traffic_light_s
{
  struct led_s red;
  struct led_s orange;
  struct led_s green;
  int pin_count;
  int pins[6];
  int mode;
  int duration;
};

void light_init(struct traffic_light_s *l)
{
  light_map_pins(l, &l->red);
  light_map_pins(l, &l->orange);
  light_map_pins(l, &l->green);
  
  light_off(l);
}

struct traffic_light_s * control_current(struct control_s * control)
{
  return &control->lights[control->current];
}

/*
 * let the traffic light turn the led for the selected mode on
 * for the configured duration
 * and then turn it off again so another possibly multiplexed or charlieplexed
 * led can be turned on without unintentionaly turning on others as well
 */
void light_refresh(struct traffic_light_s *l)
{
  switch(l->mode)
  {
		case GREEN:
			light_green(l);
			break;
		case ORANGE:
			light_orange(l);
			break;
		case RED:
			light_red(l);
			break;
			
	}
	delay(l->duration);
	light_off(l);
}

/*
 * refresh all lights
 */
void refresh(struct traffic_light_s *lights){
  struct traffic_light_s *cursor = lights;
  int i = 0;
  for (i=0; i < MAXLIGHTS; i++){
    light_refresh(cursor);
    cursor++;
  }
}

/*
 * count the total of unique pins per traffic light
 * store the pin number in .pins
 * 
 * this is done to reduce the ammount of calls to pinMode while switching the light off
 */
void light_map_pins(struct traffic_light_s *l, struct led_s * led)
{
  light_map_pin(l, led->a);
  light_map_pin(l, led->c);
}

void light_map_pin(struct traffic_light_s *l, int pin)
{
  int i;
  for(i=0; i < l->pin_count; i++)
  {
    if(pin == l->pins[i])
       return;
  }
  l->pins[i] = pin;
  l->pin_count++;
}

void light_off(struct traffic_light_s * l)
{
  int i;
  for(i=0; i<l->pin_count; i++){
    pinMode(l->pins[i], INPUT);
  }
}

void light_green(struct traffic_light_s * l)
{
  pinMode(l->green.a, OUTPUT);
  pinMode(l->green.c, OUTPUT);
  digitalWrite(l->green.a, HIGH);
  digitalWrite(l->green.c, LOW);
}

void light_orange(struct traffic_light_s * l)
{
  pinMode(l->orange.a, OUTPUT);
  pinMode(l->orange.c, OUTPUT);
  digitalWrite(l->orange.a, HIGH);
  digitalWrite(l->orange.c, LOW);
}

void light_red(struct traffic_light_s * l)
{
  pinMode(l->red.a, OUTPUT);
  pinMode(l->red.c, OUTPUT);
  digitalWrite(l->red.a, HIGH);
  digitalWrite(l->red.c, LOW);
}

/*
 * This function asumes that the charlieplexed led configuration is used
 * for 5 pins total.
 * light_off(struct traffic_light_s) has no such limitation. 
 */
void lights_off()
{
  int i;
 for(i=0; i < 5 ; i++)
 {
   digitalWrite(i + PINOFFSET, LOW);

   pinMode(i + PINOFFSET, INPUT);
   
 }
}


struct traffic_light_s lights[MAXLIGHTS];
struct control_s control;

/*
 * 1) initalize control
 * 2) configure the pinlayout of each led.
 * 3) set all traffic lights to mode orange
 * 4) blink loop
 * 5) set all traffic lights to reduce
 * 6) done with setup
 */
void setup()
{
  control.lights = lights;
  control.current = 0;
	control.button_pressed = 0;
	control.button_value = 0;

	// traffic light A
  init_led(&lights[0].red, 0, 1);
  init_led(&lights[0].orange, 1, 0);
  init_led(&lights[0].green, 0, 2);

	// traffic light B
  init_led(&lights[1].red, 1, 2);
  init_led(&lights[1].orange, 2, 1);
  init_led(&lights[1].green, 2, 0);

	// traffic light C
  init_led(&lights[2].red, 2, 3);
  init_led(&lights[2].orange, 3, 2);
  init_led(&lights[2].green, 2, 4);

	// traffic light D
  init_led(&lights[3].red, 3, 4);
  init_led(&lights[3].orange, 4, 3);
  init_led(&lights[3].green, 4, 2);
  
  int i;
  for(i = 0; i < MAXLIGHTS; i++)
  {
    light_init(&lights[i]);
    lights[i].duration = 1;
    lights[i].mode = ORANGE;
  }
  blink_loop();
  for(i = 0; i < MAXLIGHTS; i++)
    {
    light_init(&lights[i]);
    lights[i].duration = 1;
    lights[i].mode = RED;
  }
}

/*
 * stop blinking after one minute, or when te button is pressed
 */
void blink_loop()
{
  unsigned long next;
  next = millis();
  int on = 1;
  while(millis() < 60000 || check_input(&control, BUTTON_PIN))
  {
    if(next < millis())
    {
      on = -1 * on;
      next = millis() + 1000;   
    }
    if(on > 0)
      refresh(lights);
    else
      lights_off();
  }
}

void loop()
{
  refresh(lights);
  if(check_input(&control, BUTTON_PIN))
    handle_input(&control);
  check_time(&control);
 
  
}

void check_time(struct control_s * control)
{
  if(millis() > control->timeout )
  {
    control->button_pressed = 0;
    if(control_current(control)->mode == ORANGE)
    {
      control_current(control)->mode = RED;
      control->timeout = RED_TIMEOUT;
    }else if(control_current(control)->mode == GREEN){
      control_current(control)->mode = ORANGE;
      control->timeout = ORANGE_TIMEOUT;
    }else{
      control_next(control);
      control_current(control)->mode = GREEN;
      control->timeout = GREEN_TIMEOUT;
    }
    control->timeout = control->timeout + millis(); 
  }
}


int check_input(struct control_s * control, int pin)
{
  if(control->button_pressed)
    return 0;
  pinMode(pin, INPUT);
  delay(1);
  control->button_value = analogRead(pin);
  if(control->button_value > 240){
    control->button_pressed = 1;
    
  }
  return 1;
}

void handle_input(struct control_s * control)
{
  if (control->button_pressed && control_current(control)->mode == GREEN)
  {
    control->timeout = millis() + ORANGE_TIMEOUT + 1000;
  }
}

void test1_loop()
{
  int x, y, z;
  for (x=0; x < MAXLIGHTS; x++)
  {
    struct traffic_light_s * l = &lights[x];
    for (y=0; y < 4; y++){
      switch(y)
      {
	case 1:
	  light_green(l);
	  break;
	case 2:
	  light_orange(l);
	  break;
	case 3:
	  light_red(l);
	  break;
	  
      }
      delay(1000);
      light_off(l);
    }
  }
}
