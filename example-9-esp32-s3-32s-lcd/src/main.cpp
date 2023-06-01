
// #define LV_CONF_INCLUDE_SIMPLE 1
// #define LV_CONF_PATH "lv_conf.h"
#include "Wire.h"
#include "Arduino.h"
#include "FT5206.h"

#include <SPI.h>
#include <SD.h>

#include <lvgl.h>


//#define LV_CONF_INCLUDE_SIMPLE

// v1.0.0 を有効にします(v0からの移行期間の特別措置です。これを書かない場合は旧v0系で動作します。)
#define LGFX_USE_V1


#include <LovyanGFX.hpp>

#define TFT_SDA 13 // SDA
#define TFT_SCL 14 // SCL
#define TFT_DC 48
#define TFT_CS 47
#define TFT_RST 12
#define TFT_BCK_LT 45

#define TFT_WIDTH 240
#define TFT_HEIGHT 280

#define CTP_INT 9
#define CTP_SDA 10
#define CTP_SCL 11


#define CONFIG_SDCARD_SCK  15
#define CONFIG_SDCARD_MISO 16
#define CONFIG_SDCARD_MOSI 5
#define CONFIG_SDCARD_CS 4


#define LED_RUN 1 

uint8_t registers[FT5206_REGISTERS];
uint16_t new_coordinates[5][2];
uint16_t old_coordinates[5][2]; 
uint8_t current_touches = 0;
uint8_t old_touches = 0;

FT5206 cts = FT5206(CTP_INT);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ TFT_WIDTH * 10 ];


class LGFX : public lgfx::LGFX_Device
{
  
  lgfx::Panel_ST7789 _panel_instance;

  lgfx::Bus_SPI _bus_instance; // SPIバスのインスタンス

  lgfx::Light_PWM _light_instance;

  // タッチスクリーンの型にあったインスタンスを用意します。(必要なければ削除)
  lgfx::Touch_FT5x06 _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436


public:
  // コンストラクタを作成し、ここで各種設定を行います。
  // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
  LGFX(void)
  {
    {                                    // バス制御の設定を行います。
      auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

      // SPIバスの設定
      
      cfg.spi_host = SPI3_HOST; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 3;                  // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read = 16000000;          // 受信時のSPIクロック
      cfg.spi_3wire = true;              // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;               // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = TFT_SCL; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = TFT_SDA; // SPIのMOSIピン番号を設定
      cfg.pin_miso = -1; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = TFT_DC;   // SPIのD/Cピン番号を設定  (-1 = disable)
                        

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = TFT_CS;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = TFT_RST;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width = TFT_WIDTH;    // 実際に表示可能な幅
      cfg.panel_height = TFT_HEIGHT;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 20;        // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits = 1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable = true;      // データ読出しが可能な場合 trueに設定
      cfg.invert = true;        // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order = true;     // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit = false;   // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared = true;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
      //    cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
      //    cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

      _panel_instance.config(cfg);
    }

    
    {                                      // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。

      cfg.pin_bl = TFT_BCK_LT;     // バックライトが接続されているピン番号
      cfg.invert = false;  // バックライトの輝度を反転させる場合 true
      cfg.freq = 44100;    // バックライトのPWM周波数
      cfg.pwm_channel = 7; // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
    }
  
{ // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;    // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max      = TFT_WIDTH;  // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min      = 0;    // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max      = TFT_HEIGHT;  // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int    =CTP_INT;   // INTが接続されているピン番号
      cfg.bus_shared = false; // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0;// 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

// I2C接続の場合
      cfg.i2c_port = 0;      // 使用するI2Cを選択 (0 or 1)
      cfg.i2c_addr = 0x15;   // I2Cデバイスアドレス番号
      cfg.pin_sda  = CTP_SDA;     // SDAが接続されているピン番号
      cfg.pin_scl  = CTP_SCL;     // SCLが接続されているピン番号
      cfg.freq = 400000;     // I2Cクロックを設定

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }


    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

// 準備したクラスのインスタンスを作成します。
LGFX tft;

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   //tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
   tft.endWrite();

   lv_disp_flush_ready( disp );
}


static void btn_event_cb(lv_event_t * event)						//事件回调函数
{
	printf("Clicked\n");
}

void lv_example_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_align(btn, LV_ALIGN_CENTER, 0,0);
   lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_PRESSED, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
   uint16_t touchX, touchY;

   bool touched =tft.getTouch(&touchX, &touchY);

   if(!touched)
   {
    // Serial.print( "no touched " );
      data->state = LV_INDEV_STATE_REL;
   }
   else
   {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;

      Serial.print( "Data x " );
      Serial.println( touchX );

      Serial.print( "Data y " );
      Serial.println( touchY );
   }
}


void printDirectory(File dir, int numTabs)
{
    while (true)
    {

        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
            // time_t cr = entry.getCreationTime();
            // time_t lw = entry.getLastWrite();
            // struct tm *tmstruct = localtime(&cr);
            // Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
            //  tmstruct = localtime(&lw);
            // Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        }
        entry.close();
    }
}

void SDCARD_Init()
{

    // // keep checking the SD reader for valid SD card/format
    delay(100);
    Serial.println("init SD Card");

    SPI.begin(CONFIG_SDCARD_SCK, CONFIG_SDCARD_MISO, CONFIG_SDCARD_MOSI, -1);
    // while (!SD.begin(SD_CS, SPI, 2000000))
    // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 2000000))
    // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
    while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
    // while (!SD.begin(CONFIG_SDCARD_CS))
    {
        Serial.println("init SD Card Failed");
      
    }



    File root = SD.open("/");

    printDirectory(root, 0);

    root.close();

    
}


void setup(void)
{
  // SPIバスとパネルの初期化を実行すると使用可能になります。

  Serial.println("test tft lvgl start");
  Serial.begin(115200);

  //SDCARD_Init();
  delay(1000);
  Serial.println("test tft lvgl");

  // pinMode(9,OUTPUT);

  //  digitalWrite(9, 0);
  //  delay(10);
  //    digitalWrite(9, 1);
  //  delay(50);

    pinMode(LED_RUN,OUTPUT);
//      Wire.begin(CTP_SDA, CTP_SCL);
// cts.begin(SAFE);
// cts.setTouchLimit(1);//from 1 to 5


  tft.init();
     tft.setRotation(1);
   tft.setBrightness(255);

   lv_init();
   lv_disp_draw_buf_init( &draw_buf, buf, NULL, TFT_WIDTH * 10 );
   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init(&disp_drv);

   /*Change the following line to your display resolution*/
   disp_drv.hor_res = TFT_HEIGHT;
   disp_drv.ver_res = TFT_WIDTH;
   disp_drv.flush_cb = my_disp_flush;
   disp_drv.draw_buf = &draw_buf;
   lv_disp_drv_register(&disp_drv);

   /*Initialize the (dummy) input device driver*/
   static lv_indev_drv_t indev_drv;
   lv_indev_drv_init(&indev_drv);
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = my_touchpad_read;
   lv_indev_drv_register(&indev_drv);

   lv_example_get_started_1();
 // lv_demo_widgets();

}

void loop(void)
{
        digitalWrite(LED_RUN, (millis() / 1000) % 2);

   lv_timer_handler(); /* let the GUI do its work */


  // if (cts.touched()){

  //   uint8_t i;
  //   uint16_t x,y;
  //   cts.getTSregisters(registers);
  //   current_touches = cts.getTScoordinates(new_coordinates, registers);
  //   if (current_touches < 1) return;


  //   for (i = 1; i <= current_touches; i++){// mark touches on screen
  //     x = new_coordinates[i-1][0];
  //     y = new_coordinates[i-1][1];
 
  //  Serial.printf("X:%d Y:%d\n", x, y);
  //     old_coordinates[i-1][0] = x;
  //     old_coordinates[i-1][1] = y;
  //   }
  //   old_touches = current_touches;

  //   cts.rearmISR();
  // }



}
