#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif

#include "lcm_drv.h"
#ifdef BUILD_LK
    //#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <mach/mt_gpio.h>
#endif
#define AUXADC_LCM_VOLTAGE_CHANNEL     0
#include <cust_adc.h>    	
#define MIN_VOLTAGE (0)	//++++rgk bug-id:no modify by yangjuwei 20140401
#define MAX_VOLTAGE (300)	//++++rgk bug-id:no modify by yangjuwei 20140401

#define FRAME_WIDTH                                          (720)
#define FRAME_HEIGHT                                         (1280)

//#define GPIO_LCD_ENN GPIO_LCD_BIAS_ENP_PIN //sophiarui
//#define GPIO_LCD_ENP GPIO_LCD_BIAS_ENN_PIN //sophiarui

#define REGFLAG_DELAY                                         0XFFE
#define REGFLAG_END_OF_TABLE                      			 0x1FF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE                                    0

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    			(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 				(lcm_util.udelay(n))
#define MDELAY(n) 				(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)		lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)			lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)							lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
#ifndef BUILD_LK
//extern atomic_t ESDCheck_byCPU;
#endif

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

	memset(params, 0, sizeof(LCM_PARAMS));
	
	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;


#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
#endif
	
	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Video mode setting		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 8;// 3    2
		params->dsi.vertical_backporch					= 18;// 20   1
		params->dsi.vertical_frontporch					= 20; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20;// 50  2
		params->dsi.horizontal_backporch				= 50;//90
		params->dsi.horizontal_frontporch				= 60;//90
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
//		params->dsi.ssc_disable                         = 1;
		params->dsi.HS_TRAIL                             = 15;
		/*ui = 1000/(dis_params->PLL_CLOCK*2) + 0x01; 
		cycle_time = 8000/(dis_params->PLL_CLOCK*2) + 0x01;
		HS_TRAIL = (0x04*ui + 0x50)/cycle_time;*/
	//params->dsi.LPX=8; 

	// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_select=1;
		//params->dsi.PLL_CLOCK  = LCM_DSI_6589_PLL_CLOCK_377;
		params->dsi.PLL_CLOCK=220;//208//270		
		//params->dsi.noncont_clock=1;
		//params->dsi.noncont_clock_period=2;
		//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	

	//	params->dsi.fbk_div =7;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		//params->dsi.compatibility_for_nvk = 1;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
		//params->dsi.esd_check_enable = 1;
	//	params->dsi.customization_esd_check_enable = 1;
//		params->dsi.lcm_esd_check_table[0].cmd			= 0x0a;
//		params->dsi.lcm_esd_check_table[0].count		= 1;
//		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

}

//static unsigned int vcom = 0x30;
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	
	for(i = 0; i < count; i++) 
	{   
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd)
		{
			case REGFLAG_DELAY :
				MDELAY(table[i].count);
				break;
			case REGFLAG_END_OF_TABLE :
				break;
			/*case 0xd9 :
				table[i].para_list[0] = vcom;
				vcom +=2;
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				break;*/
			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF,3,{0x98,0x81,0x03}},

	{0x01,1,{0x00}},

	{0x02,1,{0x00}},

{0x03,1,{0x73}},

{0x04,1,{0x00}},

{0x05,1,{0x00}},

{0x06,1,{0x0C}},

	{0x07,1,{0x00}},

{0x08,1,{0x00}},

{0x09,1,{0x01}},

{0x0a,1,{0x01}},

{0x0b,1,{0x01}},

{0x0c,1,{0x01}},

{0x0d,1,{0x01}},

{0x0e,1,{0x01}},

{0x0f,1,{0x20}},

{0x10,1,{0x20}},

	{0x11,1,{0x00}},

	{0x12,1,{0x00}},

{0x13,1,{0x02}},

	{0x14,1,{0x00}},

{0x15,1,{0x05}},

	{0x16,1,{0x00}},

	{0x17,1,{0x00}},

	{0x18,1,{0x00}},

	{0x19,1,{0x00}},

{0x1a,1,{0x00}},

{0x1b,1,{0x00}},

{0x1c,1,{0x00}},

{0x1d,1,{0x00}},

{0x1e,1,{0x44}},

{0x1f,1,{0xC1}},

{0x20,1,{0x0A}},

{0x21,1,{0x03}},

{0x22,1,{0x0A}},

{0x23,1,{0x03}},

{0x24,1,{0x8C}},

{0x25,1,{0x8C}},

	{0x26,1,{0x00}},

	{0x27,1,{0x00}},

{0x28,1,{0x3B}},

	{0x29,1,{0x03}},

{0x2a,1,{0x00}},

{0x2b,1,{0x00}},

{0x2c,1,{0x00}},	//ori06

{0x2d,1,{0x00}},

{0x2e,1,{0x00}},

{0x2f,1,{0x00}},

	{0x30,1,{0x00}},

	{0x31,1,{0x00}},

	{0x32,1,{0x00}},

	{0x33,1,{0x00}},

{0x34,1,{0x43}},

{0x35,1,{0x00}},		//05

{0x36,1,{0x03}},

	{0x37,1,{0x00}},

{0x38,1,{0x00}},		//00

	{0x39,1,{0x00}},

{0x3a,1,{0x00}},

{0x3b,1,{0x00}},

{0x3c,1,{0x00}},

{0x3d,1,{0x00}},

{0x3e,1,{0x00}},

{0x3f,1,{0x00}},

	{0x40,1,{0x00}},

	{0x41,1,{0x00}},

	{0x42,1,{0x00}},

	{0x43,1,{0x00}},

	{0x44,1,{0x00}},

	{0x50,1,{0x01}},

	{0x51,1,{0x23}},

	{0x52,1,{0x45}},

	{0x53,1,{0x67}},

	{0x54,1,{0x89}},

{0x55,1,{0xab}},

	{0x56,1,{0x01}},

	{0x57,1,{0x23}},

	{0x58,1,{0x45}},

	{0x59,1,{0x67}},

{0x5a,1,{0x89}},

{0x5b,1,{0xab}},

{0x5c,1,{0xcd}},

{0x5d,1,{0xef}},

{0x5e,1,{0x11}},

{0x5f,1,{0x02}},

{0x60,1,{0x00}},

	{0x61,1,{0x0C}},

	{0x62,1,{0x0D}},

	{0x63,1,{0x0E}},

	{0x64,1,{0x0F}},

{0x65,1,{0x02}},

{0x66,1,{0x02}},

{0x67,1,{0x02}},

	{0x68,1,{0x02}},

{0x69,1,{0x02}},

{0x6a,1,{0x02}},

{0x6b,1,{0x02}},

{0x6c,1,{0x02}},

{0x6d,1,{0x02}},

{0x6e,1,{0x05}},

{0x6f,1,{0x05}},

{0x70,1,{0x05}},

{0x71,1,{0x05}},

{0x72,1,{0x01}},

{0x73,1,{0x06}},

{0x74,1,{0x07}},

{0x75,1,{0x12}},

{0x76,1,{0x00}},

{0x77,1,{0x0C}},

{0x78,1,{0x0D}},

{0x79,1,{0x0E}},

{0x7a,1,{0x0F}},

{0x7b,1,{0x02}},

{0x7c,1,{0x02}},

{0x7d,1,{0x02}},

{0x7e,1,{0x02}},

{0x7f,1,{0x02}},

	{0x80,1,{0x02}},

	{0x81,1,{0x02}},

	{0x82,1,{0x02}},

	{0x83,1,{0x02}},

{0x84,1,{0x05}},

{0x85,1,{0x05}},

{0x86,1,{0x05}},

{0x87,1,{0x05}},

{0x88,1,{0x01}},

{0x89,1,{0x06}},

{0x8A,1,{0x07}},

	{0xFF,3,{0x98,0x81,0x04}},
	{0x00,1,{0x80}},
	{0xb5,1,{0x06}},
	{0x31,1,{0x75}},

	{0x6C,1,{0x15}},

{0x6E,1,{0x1A}},              //di_pwr_reg=0 for power mode 2A //VGH clamp 18V

	{0x6F,1,{0x57}},                // reg vcl + pumping ratio VGH=4x VGL=-2.5x
	 
	{0x3A,1,{0xA4}},

{0x8D,1,{0x20}},               //VGL clamp -10V

	{0x87,1,{0xBA}},

	{0x26,1,{0x76}},

{0xB2,1,{0xD1}},

//{0x88,1,{0x0B}},

	{0xFF,3,{0x98,0x81,0x01}},

	{0x22,1,{0x0A}},

	{0x31,1,{0x00}},

{0x53,1,{0x72}},             //VCOM1

{0x55,1,{0x77}},               //VCOM2

	{0x50,1,{0xA6}},

	{0x51,1,{0xA6}},

{0x60,1,{0x20}},               //SDT

	{0xA0,1,{0x08}},

{0xA1,1,{0x10}},               //VP251

{0xA2,1,{0x1F}},               //VP247

{0xA3,1,{0x13}},               //VP243

{0xA4,1,{0x15}},               //VP239

{0xA5,1,{0x27}},               //VP231

{0xA6,1,{0x1B}},               //VP219

{0xA7,1,{0x1D}},               //VP203

{0xA8,1,{0x74}},             //VP175

{0xA9,1,{0x1B}},              //VP144

{0xAA,1,{0x2B}},               //VP111

{0xAB,1,{0x5F}},               //VP80

{0xAC,1,{0x18}},               //VP52

{0xAD,1,{0x16}},               //VP36

{0xAE,1,{0x49}},               //VP24

{0xAF,1,{0x1F}},               //VP16

{0xB0,1,{0x25}},               //VP12

{0xB1,1,{0x44}},               //VP8

{0xB2,1,{0x54}},               //VP4

{0xB3,1,{0x39}},               //VP0

	{0xC0,1,{0x08}},

{0xC1,1,{0x1A}},               //VN251

{0xC2,1,{0x22}},               //VN247

{0xC3,1,{0x0F}},               //VN243

{0xC4,1,{0x11}},               //VN239

{0xC5,1,{0x22}},               //VN231

{0xC6,1,{0x17}},               //VN219

{0xC7,1,{0x19}},               //VN203

{0xC8,1,{0x66}},               //VN175

	{0xC9,1,{0x1C}},

{0xCA,1,{0x25}},               //VN111

{0xCB,1,{0x58}},               //VN80

{0xCC,1,{0x1A}},               //VN52

{0xCD,1,{0x1A}},               //VN36

{0xCE,1,{0x4E}},               //VN24

{0xCF,1,{0x22}},               //VN16

{0xD0,1,{0x2B}},               //VN12

{0xD1,1,{0x42}},               //VN8

{0xD2,1,{0x52}},               //VN4

{0xD3,1,{0x39}},               //VN0

	{0xFF,3,{0x98,0x81,0x00}},

	{0x35,1,{0x00}},

	{0x11,1,{0x00}},

	{REGFLAG_DELAY, 120, {}},

	{0x29,1,{0x00}},

	{REGFLAG_DELAY, 20, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}},
};

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h> 
#include <platform/mt_pmic.h>

#define TPS65132_SLAVE_ADDR_WRITE  0x7C  
static struct mt_i2c_t TPS65132_i2c;
static int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;
    write_data[0]= addr;
    write_data[1] = value;
    TPS65132_i2c.id = 1;//I2C2;
    TPS65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
    TPS65132_i2c.mode = ST_MODE;
    TPS65132_i2c.speed = 100;
    len = 2;
    ret_code = i2c_write(&TPS65132_i2c, write_data, len);
    return ret_code;
}
#else
extern int tps65132_write_bytes(unsigned char addr, unsigned char value);
#endif
static void lcm_init(void)
{
	int ret=0;
    lcm_util.set_gpio_mode(GPIO_LCD_ENN, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCD_ENN, GPIO_DIR_OUT); 
	//lcm_util.set_gpio_pull_enable(LCD_LDO_ENP_GPIO_PIN, GPIO_PULL_DISABLE); 

	lcm_util.set_gpio_mode(GPIO_LCD_ENP, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCD_ENP, GPIO_DIR_OUT); 
	//lcm_util.set_gpio_pull_enable(LCD_LDO_ENN_GPIO_PIN, GPIO_PULL_DISABLE); 
#ifdef BUILD_LK
	ret=TPS65132_write_byte(0x0, 0xC); //5.2V
	if(ret) 	
		printf("[LK]ili9881c_txd ----tps6132----ret=%0x--i2c write error----\n",ret);		
	else
		printf("[LK]ili9881c_txd ----tps6132----ret=%0x--i2c write success----\n",ret);			
#else
	ret=tps65132_write_bytes(0x0, 0xC);
	if(ret<0)
		printk("[KERNEL]ili9881c_txd ----tps6132---ret=%0x-- i2c write error-----\n",ret);
	else
		printk("[KERNEL]ili9881c_txd ----tps6132---ret=%0x-- i2c write success-----\n",ret);
#endif
	MDELAY(20);
#ifdef BUILD_LK
	ret=TPS65132_write_byte(0x1, 0xC); //-5.2v
	if(ret) 	
		dprintf(0, "[LK]ili9881c_txd ----tps6132----ret=%0x--i2c write error----\n",ret);		
	else
		dprintf(0, "[LK]ili9881c_txd ----tps6132----ret=%0x--i2c write success----\n",ret);	 
#else
	ret=tps65132_write_bytes(0x1, 0xC);
	if(ret<0)
		printk("[KERNEL]ili9881c_txd ----tps6132---ret=%0x-- i2c write error-----\n",ret);
	else
		printk("[KERNEL]ili9881c_txd ----tps6132---ret=%0x-- i2c write success-----\n",ret);
#endif
	lcm_util.set_gpio_out(GPIO_LCD_ENP , 1);//power on +5
	MDELAY(10);
	lcm_util.set_gpio_out(GPIO_LCD_ENN , 1);//power on -5
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(2); 
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);	
	MDELAY(120);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_update(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height)
{
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;

    unsigned char x0_MSB = ((x0>>8)&0xFF);
    unsigned char x0_LSB = (x0&0xFF);
    unsigned char x1_MSB = ((x1>>8)&0xFF);
    unsigned char x1_LSB = (x1&0xFF);
    unsigned char y0_MSB = ((y0>>8)&0xFF);
    unsigned char y0_LSB = (y0&0xFF);
    unsigned char y1_MSB = ((y1>>8)&0xFF);
    unsigned char y1_LSB = (y1&0xFF);

    unsigned int data_array[16];

    data_array[0]= 0x00053902;
    data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
    data_array[2]= (x1_LSB);
    dsi_set_cmdq(&data_array, 3, 1);
    
    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(&data_array, 3, 1);
    
    data_array[0]= 0x00290508;
    dsi_set_cmdq(&data_array, 1, 1);
    
    data_array[0]= 0x002c3909;
    dsi_set_cmdq(&data_array, 1, 0);


}

static struct LCM_setting_table lcm_exit_sleep_mode_setting[] = {
	{0x11,1,{0x00}},   // Sleep out 

	{REGFLAG_DELAY, 120, {}},

	{0x29,1,{0x00}},   // Display on 

	{REGFLAG_DELAY, 20, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}},	
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	{0x28,1,{0x00}},   // Display off 
	
	{REGFLAG_DELAY, 20, {}},
	
	{0x10,1,{0x00}},   // Enter Sleep mode 
	
	{REGFLAG_DELAY, 120, {}},

  	{REGFLAG_END_OF_TABLE, 0x00, {}},
};

static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	
	SET_RESET_PIN(1);
	MDELAY(1); 
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);	
	MDELAY(120);
	lcm_util.set_gpio_out(GPIO_LCD_ENP , 0);//power on +5
	MDELAY(10);
	lcm_util.set_gpio_out(GPIO_LCD_ENN , 0);//power on -5
	MDELAY(10); 
}

static void lcm_resume(void)
{
    lcm_init();
}


static unsigned int lcm_compare_id(void)
{
	
	int array[4];
	char buffer[3];
	char id_high=0;
	char id_low=0;
	int id=0;

	SET_RESET_PIN(1);
	MDELAY(20); 
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(100);

	//{0x39, 0xFF, 5, { 0xFF,0x98,0x06,0x04,0x01}}, // Change to Page 1 CMD
	array[0] = 0x00043902;
	array[1] = 0x018198FF;
	dsi_set_cmdq(array, 2, 1);

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x00, &buffer[0], 1);  //0x98

	id = buffer[0];

	
#ifdef BUILD_LK
	  
		printf("jinmin ili9881c_dsi_vdo_tcl_hd720_boe_ips [lk]=%d %d %d \n", buffer[0],buffer[1], buffer[2]);
		printf("id =0x%x\n", id);
#else
		printk("jinmin ili9881c_dsi_vdo_tcl_hd720_boe_ips [kernel]=%d %d %d \n", buffer[0],buffer[1], buffer[2]);
		printk("id =0x%x\n", id);
#endif

	return (0x98 == id)?1:0;

}


// zhoulidong  add for lcm detect (start)
static unsigned int rgk_lcm_compare_id(void)
{
    int data[4] = {0,0,0,0};
    int res = 0;
    int rawdata = 0;
    int lcm_vol = 0;

#ifdef AUXADC_LCM_VOLTAGE_CHANNEL
    res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL,data,&rawdata);
    if(res < 0)
    { 
	#ifdef BUILD_LK
	printf("[adc_uboot]: jinmin get data error\n");
	#endif
	return 0;
		   
    }
#endif

    lcm_vol = data[0]*1000+data[1]*10;

    #ifdef BUILD_LK
    printf("[adc_uboot]: jinmin lcm_vol= %d\n",lcm_vol);
    #endif
	
    if (lcm_vol>=MIN_VOLTAGE &&lcm_vol <= MAX_VOLTAGE)// && lcm_compare_id())
    {
	return 1;
    }

    return 0;



	//MDELAY(10);
	//printk("ghj ########## 9806e_gp lcd_id=%x,\r\n",id);     
 




}
static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	unsigned int id ,id1,id2,id3,id4,id5,id6,id7 = 0;  
	unsigned char buffer[6];   
	unsigned int data_array[16];   
	static int lcm_id;  
	UDELAY(600);      
	data_array[0] = 0x00013700;// read id return two byte,version and id  
	dsi_set_cmdq(data_array, 1, 1);  
	//MDELAY(10);
	read_reg_v2(0x0A, buffer, 1);    // A1  
	id = buffer[0]; //we only need ID     
	//printk("ghj ########## 9806e_gp lcd_id=%x,\r\n",id);     
	if(id ==0x9c)     
	{    
	return 0;   
	}      
	else      
	{         
	return 1; //TRUE     
 
	}
 #endif

}


static unsigned int lcm_esd_recover(void)
{
    lcm_init();
    lcm_resume();

    return TRUE;
}

//add by yangjuwei 
static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
			int array[4];
			char buf[3];
			char id_high=0;
			char id_low=0;
			int id=0;
		
	    array[0] = 0x00043902;
		array[1] = 0x018198FF;
		dsi_set_cmdq(array, 2, 1);
	
		array[0] = 0x00013700;
		dsi_set_cmdq(array, 1, 1);
//		atomic_set(&ESDCheck_byCPU,1);
		read_reg_v2(0x00, &buf[0], 1);  //0x98
//		atomic_set(&ESDCheck_byCPU,0);
		id = buf[0];	
			return (0x98 == id)?1:0;
#else
	return 0;
#endif
}

LCM_DRIVER ili9881c_dsi_vdo_djn_lcm_drv = 
{
	.name			= "ili9881c_dsi_vdo_djn",
	.set_util_funcs		= lcm_set_util_funcs,
	.get_params		= lcm_get_params,
	.init			= lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = rgk_lcm_compare_id,
	//.ata_check		= lcm_ata_check,
    .esd_check      = lcm_esd_check,
    .esd_recover    = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	//.set_backlight	= lcm_setbacklight,
	//.update		= lcm_update,
#endif
};

