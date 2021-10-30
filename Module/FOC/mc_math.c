
#include "mc_math.h"
#include "mc_type.h"

/* Private macro -------------------------------------------------------------*/
#define SIN_COS_TABLE {\
    0x0000,0x00C9,0x0192,0x025B,0x0324,0x03ED,0x04B6,0x057F,\
    0x0648,0x0711,0x07D9,0x08A2,0x096A,0x0A33,0x0AFB,0x0BC4,\
    0x0C8C,0x0D54,0x0E1C,0x0EE3,0x0FAB,0x1072,0x113A,0x1201,\
    0x12C8,0x138F,0x1455,0x151C,0x15E2,0x16A8,0x176E,0x1833,\
    0x18F9,0x19BE,0x1A82,0x1B47,0x1C0B,0x1CCF,0x1D93,0x1E57,\
    0x1F1A,0x1FDD,0x209F,0x2161,0x2223,0x22E5,0x23A6,0x2467,\
    0x2528,0x25E8,0x26A8,0x2767,0x2826,0x28E5,0x29A3,0x2A61,\
    0x2B1F,0x2BDC,0x2C99,0x2D55,0x2E11,0x2ECC,0x2F87,0x3041,\
    0x30FB,0x31B5,0x326E,0x3326,0x33DF,0x3496,0x354D,0x3604,\
    0x36BA,0x376F,0x3824,0x38D9,0x398C,0x3A40,0x3AF2,0x3BA5,\
    0x3C56,0x3D07,0x3DB8,0x3E68,0x3F17,0x3FC5,0x4073,0x4121,\
    0x41CE,0x427A,0x4325,0x43D0,0x447A,0x4524,0x45CD,0x4675,\
    0x471C,0x47C3,0x4869,0x490F,0x49B4,0x4A58,0x4AFB,0x4B9D,\
    0x4C3F,0x4CE0,0x4D81,0x4E20,0x4EBF,0x4F5D,0x4FFB,0x5097,\
    0x5133,0x51CE,0x5268,0x5302,0x539B,0x5432,0x54C9,0x5560,\
    0x55F5,0x568A,0x571D,0x57B0,0x5842,0x58D3,0x5964,0x59F3,\
    0x5A82,0x5B0F,0x5B9C,0x5C28,0x5CB3,0x5D3E,0x5DC7,0x5E4F,\
    0x5ED7,0x5F5D,0x5FE3,0x6068,0x60EB,0x616E,0x61F0,0x6271,\
    0x62F1,0x6370,0x63EE,0x646C,0x64E8,0x6563,0x65DD,0x6656,\
    0x66CF,0x6746,0x67BC,0x6832,0x68A6,0x6919,0x698B,0x69FD,\
    0x6A6D,0x6ADC,0x6B4A,0x6BB7,0x6C23,0x6C8E,0x6CF8,0x6D61,\
    0x6DC9,0x6E30,0x6E96,0x6EFB,0x6F5E,0x6FC1,0x7022,0x7083,\
    0x70E2,0x7140,0x719D,0x71F9,0x7254,0x72AE,0x7307,0x735E,\
    0x73B5,0x740A,0x745F,0x74B2,0x7504,0x7555,0x75A5,0x75F3,\
    0x7641,0x768D,0x76D8,0x7722,0x776B,0x77B3,0x77FA,0x783F,\
    0x7884,0x78C7,0x7909,0x794A,0x7989,0x79C8,0x7A05,0x7A41,\
    0x7A7C,0x7AB6,0x7AEE,0x7B26,0x7B5C,0x7B91,0x7BC5,0x7BF8,\
    0x7C29,0x7C59,0x7C88,0x7CB6,0x7CE3,0x7D0E,0x7D39,0x7D62,\
    0x7D89,0x7DB0,0x7DD5,0x7DFA,0x7E1D,0x7E3E,0x7E5F,0x7E7E,\
    0x7E9C,0x7EB9,0x7ED5,0x7EEF,0x7F09,0x7F21,0x7F37,0x7F4D,\
    0x7F61,0x7F74,0x7F86,0x7F97,0x7FA6,0x7FB4,0x7FC1,0x7FCD,\
    0x7FD8,0x7FE1,0x7FE9,0x7FF0,0x7FF5,0x7FF9,0x7FFD,0x7FFE}  // 最大值32766

#define SIN_MASK        0x0300u
#define U0_90           0x0200u
#define U90_180         0x0300u
#define U180_270        0x0000u
#define U270_360        0x0100u
	
// 16位有符号数，表示-1到0或0到1之间的小数
const int16_t h_sin_cos_table[256] = SIN_COS_TABLE;
	
#define divSQRT_3 (int32_t)0x49E6    /* 1/sqrt(3) in q1.15 format=0.5773315*/
	
#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__( ( section ( ".ccmram" ) ) )
#endif
#endif

// Clarke变换问题不是很大，主要注意两点：
// 1.Clark变换中前面×了系数2/3
// 2.建立的坐标系alpha和常见的一样，但是beta坐标系是朝下的，所以和一般的结果差一个负号
alphabeta_t mc_clark(ab_t input)
{
	alphabeta_t output;

	int32_t a_divSQRT3_tmp, b_divSQRT3_tmp ;
	int32_t wbeta_tmp;
	int16_t hbeta_tmp;

	/* qIalpha = qIas*/
	output.alpha = input.a;  // 注意这里的clarke变换×了系数2/3

	a_divSQRT3_tmp = divSQRT_3 * ( int32_t )input.a;

	b_divSQRT3_tmp = divSQRT_3 * ( int32_t )input.b;

	/*qIbeta = -(2*qIbs+qIas)/sqrt(3)*/   // 这里是负的，也就是beta轴是朝下的
	wbeta_tmp = ( -( a_divSQRT3_tmp ) - ( b_divSQRT3_tmp ) -
	( b_divSQRT3_tmp ) ) >> 15;  
	// 这里>>15是因为上面是两个Q15格式的数据进行运算，结果是Q30格式，需要重新转换为Q15格式


	/* Check saturation of Ibeta */
	if ( wbeta_tmp > INT16_MAX )  // INT16_MAX = 32767
	{
		hbeta_tmp = INT16_MAX;
	}
	else if ( wbeta_tmp < ( -32768 ) )
	{
		hbeta_tmp = ( -32768 );
	}
	else
	{
		hbeta_tmp = ( int16_t )( wbeta_tmp );
	}

	output.beta = hbeta_tmp;

	// 这里的限幅是干什么的？把-32768限幅成-32767?
	if ( output.beta == ( int16_t )( -32768 ) )
	{
		output.beta = -32767;
	}

	return ( output );
}

// Park变化问题比较大，目前还没有弄明白
// 解决！终于明白了这里的Park变换为什么不对了！
// 坐标系的定义：https://www.cnblogs.com/neriq/p/14800876.html
qd_t mc_park(alphabeta_t input, int16_t theta)
{
	qd_t output;
	int32_t d_tmp_1, d_tmp_2, q_tmp_1, q_tmp_2;
	trig_components local_vector_components;
	int32_t wqd_tmp;
	int16_t hqd_tmp;

	// 计算输入的角度的sin和cos值
	local_vector_components = mc_trig_functions(theta);

	/*No overflow guaranteed  无溢出保证*/
	q_tmp_1 = input.alpha * ( int32_t )local_vector_components.h_cos;

	/*No overflow guaranteed  无溢出保证*/
	q_tmp_2 = input.beta * ( int32_t )local_vector_components.h_sin;

	/*Iq component in Q1.15 Format */
	wqd_tmp = ( q_tmp_1 - q_tmp_2 ) >> 15;  // 这里就是在算iq
	// 注意这里>>15为还是因为前面的结果是两个Q15格式的运算，结果是Q30格式，需要重新转换为Q15格式
	
	/* Check saturation of Iq */
	if ( wqd_tmp > INT16_MAX )
	{
		hqd_tmp = INT16_MAX;
	}
	else if ( wqd_tmp < ( -32768 ) )
	{
		hqd_tmp = ( -32768 );
	}
	else
	{
		hqd_tmp = ( int16_t )( wqd_tmp );
	}

	output.q = hqd_tmp;

	if ( output.q == ( int16_t )( -32768 ) )
	{
		output.q = -32767;
	}

	/*No overflow guaranteed*/
	d_tmp_1 = input.alpha * ( int32_t )local_vector_components.h_sin;

	/*No overflow guaranteed*/
	d_tmp_2 = input.beta * ( int32_t )local_vector_components.h_cos;

	wqd_tmp = ( d_tmp_1 + d_tmp_2 ) >> 15;

	/* Check saturation of Id */
	if ( wqd_tmp > INT16_MAX )
	{
		hqd_tmp = INT16_MAX;
	}
	else if ( wqd_tmp < ( -32768 ) )
	{
		hqd_tmp = ( -32768 );
	}
	else
	{
		hqd_tmp = ( int16_t )( wqd_tmp );
	}

	output.d = hqd_tmp;

	if ( output.d == ( int16_t )( -32768 ) )
	{
		output.d = -32767;
	}

	return (output);
}
	
/**
  * @brief 反park变换:将同步旋转坐标系下的vqd，变换为静止坐标系的v_alpha,v_beta
  * @param volt_input: vqd
  * @param theta: 电角度值
  */
alphabeta_t mc_rev_park(qd_t volt_input, int16_t theta)
{
	int32_t q_v_alpha_tmp1, q_v_alpha_tmp2, q_v_beta_tmp1, q_v_beta_tmp2;
	trig_components local_vector_components;
	alphabeta_t volt_output;
	
	local_vector_components = mc_trig_functions(theta);

	q_v_alpha_tmp1 = volt_input.q * ( int32_t )local_vector_components.h_cos;
	q_v_alpha_tmp2 = volt_input.d * ( int32_t )local_vector_components.h_sin;
	
	volt_output.alpha = ( int16_t )( ( ( q_v_alpha_tmp1 ) + ( q_v_alpha_tmp2 ) ) >> 15 );
	

	q_v_beta_tmp1 = volt_input.q * ( int32_t )local_vector_components.h_sin;
	q_v_beta_tmp2 = volt_input.d * ( int32_t )local_vector_components.h_cos;
	
	volt_output.beta = ( int16_t )( ( q_v_beta_tmp2 - q_v_beta_tmp1 ) >> 15 );
	
	return (volt_output);
}

/**
  * @brief 根据输入的电角度，计算三角正弦和余弦
  * @param h_angle: 电角度值
  */
trig_components mc_trig_functions( int16_t h_angle )
{
  int32_t shindex;
  uint16_t uhindex;

  trig_components local_components;

  /* 10 bit index computation  */
	// 这里也相当于把-180到+179.99的角度表示进行了平移，变成了0到359.99的角度表示
  shindex = ( ( int32_t )32768 + ( int32_t )h_angle );  // int16在-32768——32767之间，这里就是全部化为正数
  uhindex = ( uint16_t )shindex;
  uhindex /= ( uint16_t )64;  // 65536/64 = 1024

  // 65536/4=16384,也就是把360度按照90度划分，一个90度范围最大值对应16384
  // 而90度的正弦表一共生成了256个，上面除以64，那么256*64=16384
  // 那么上面除以64取整，结果应该在0-256*4之间，也就是0-1024之间，最大是0x0400
  // ST为了方便判断属于哪个90度范围区间(360度按90度一个区间划分一共4个区间）,故意这样设置。
	
            /******* 根据uhindex的结果的不同，可以讨论如下：  *******/
/*
  // 1：结果在0-256之间，也就是最小0，最大0x00ff(255)，此时偏移后的角度是0-90度。
	  由于角度值进来就+32768（+180度）进行了偏移，所以此时对应原来-180~+180的角度应该
	  是-180~-90度，根据周期性其三角函数值等于180~270度的函数值。index & SIN_MASK结果是0，
	 恰好对应程序中的U180_270分支。并且此时index的低8位就代表了当前角度在180-270度中的位置。
	 这里要把180-270度的函数值转换到0-90度中求解，根据sin图像可知此时的sin/cos都直接加负号即可
	
  // 2: 结果在256-512之间，也就是最小0x0100(256),最大0x01ff(511)，此时偏移后的角度是90-180度。
	  对应原来-180~+180的角度应该是那么-90~0度，根据周期性其三角函数值等于270~360度的函数值。
	  index & SIN_MASK结果是0x1000，恰好对应程序中的U270_360分支。并且此时index的低8位就代表
	  了当前角度在270~360度中的位置。这里要把270~360度的函数值转换到0-90度中求解，
	  根据sin图像可知此时的角度索引需要交换，并且sin函数值取反，cos函数值符号不变。
	  
  // 3: 结果在512-768之间，也就是最小0x0200(512),最大0x02ff(767)，此时偏移后的角度是180-270度。
	  对应原来-180~+180的角度应该是那么0~90度，根据周期性其三角函数值等于0~90度的函数值。
	  index & SIN_MASK结果是0x2000，恰好对应程序中的U0_90分支。并且此时index的低8位就代表
	  了当前角度在0~90度中的位置。这里直接在0-90度中求解即可，而且sin和cos无须做任何变换
	  
  // 4: 结果在768-1024之间，也就是最小0x0300(768),最大0x03ff(1023)，此时偏移后的角度是270-360度。
	  对应原来-180~+180的角度应该是那么90~180度，根据周期性其三角函数值等于90~180度的函数值。
	  index & SIN_MASK结果是0x3000，恰好对应程序中的U90_180分支。并且此时index的低8位就代表
	  了当前角度在90~180度中的位置。这里要把90~180度的函数值转换到0-90度中求解，
	  根据sin图像可知此时的角度索引需要交换，并且cos函数值取反，sin函数值符号不变。	
*/
  switch ( ( uint16_t )( uhindex ) & SIN_MASK )  // 0x0300u  768
  {
    case U0_90:  // 0x0200u
      local_components.h_sin = h_sin_cos_table[( uint8_t )( uhindex )];
      local_components.h_cos = h_sin_cos_table[( uint8_t )( 0xFFu - ( uint8_t )( uhindex ) )];
      break;

    case U90_180:  // 0x0300u
      local_components.h_sin = h_sin_cos_table[( uint8_t )( 0xFFu - ( uint8_t )( uhindex ) )];
      local_components.h_cos = -h_sin_cos_table[( uint8_t )( uhindex )];
      break;

    case U180_270:  // 0x0000u
      local_components.h_sin = -h_sin_cos_table[( uint8_t )( uhindex )];
      local_components.h_cos = -h_sin_cos_table[( uint8_t )( 0xFFu - ( uint8_t )( uhindex ) )];
      break;

    case U270_360:  // 0x0100u
      local_components.h_sin =  -h_sin_cos_table[( uint8_t )( 0xFFu - ( uint8_t )( uhindex ) )];
      local_components.h_cos =  h_sin_cos_table[( uint8_t )( uhindex )];
      break;
    default:
      break;
  }
  return ( local_components );
}
