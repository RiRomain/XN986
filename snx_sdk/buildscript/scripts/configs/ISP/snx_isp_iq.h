
// cmd = 1
struct drc_arg
{
	unsigned int Enable;
	unsigned int DarALim;
	unsigned int BriALim;
	unsigned int IntVar;
	unsigned int IntMode;
	unsigned int P1Gain;
	unsigned int P2Gain;
	unsigned int P3Gain;
	unsigned int P4Gain;
	unsigned int FSeed;
	unsigned int ASeed;
	unsigned int box_h_num;
	unsigned int box_v_num;
	unsigned int Bghsov;
	unsigned int Bgvsov;
	unsigned int Bghol;
	unsigned int Bgvol;
	unsigned int Bgw2;
	unsigned int Bgh2;
	unsigned int Bgho;
	unsigned int Bgvo;
	unsigned int Bghno;
	unsigned int Bgvno;
	unsigned int Bgdelx;
	unsigned int Bgdely;
	unsigned int BGPMUH;
	unsigned int BGPMUV;
};

// cmd = 33
struct lsc_arg
{
	unsigned int Raw_Bypass_EN;
	unsigned int Enable;
	unsigned int CG;
	unsigned int Raw_B_Order;
	unsigned int Raw_GB_Order;
	unsigned int Raw_GR_Order;
	unsigned int Raw_R_Order;
	unsigned int Gain_Limit;
	unsigned int Circle_En;
	unsigned int AUTO_SET_MS;
	unsigned int CenterX;
	unsigned int CenterY;
	unsigned int Mvalue;
	unsigned int Svalue;
	unsigned int Ps;
	unsigned int Pc;
	unsigned int RGain_0;
	unsigned int RGain_1;
	unsigned int GGain_0;
	unsigned int GGain_1;
	unsigned int BGain_0;
	unsigned int BGain_1;
	unsigned int SM_En;
	unsigned int SM_N;
	unsigned int BlockEn;
	unsigned int X_Offset;
	unsigned int Y_Offset;
	unsigned int BlockMode_32;
	unsigned int BlockMode_64;
	unsigned int BlockMode_128;
	unsigned int BlockMode_256;
	unsigned int Gain_Width;
	unsigned int GG_Type;
	unsigned int Shading_Target;
	unsigned int Ratio_Sync;
	unsigned int B_Channel_Ratio;
	unsigned int G_Channel_Ratio;
	unsigned int R_Channel_Ratio;
	unsigned int Auto_Tune;
	unsigned int RSlope_0;
	unsigned int RSlope_1;
	unsigned int GSlope_0;
	unsigned int GSlope_1;
	unsigned int BSlope_0;
	unsigned int BSlope_1;
	unsigned int End_Pnt_H;
	unsigned int End_Pnt_V;
	unsigned int Block_Size;
};

// cmd = 34
struct lsc_gain_table
{
	unsigned int RGB_tbl;
	unsigned int Gain_Width;
	unsigned int Gain_Height;
	unsigned int BGT[13][17];
};

// cmd = 36
struct dpc_arg
{
	unsigned int WhiteDefectEn;
	unsigned int ClusterDPCEn;
	unsigned int BlackDefectEn;
	unsigned int Cluster3DPCEn;
	unsigned int WhtDftThd;
	unsigned int BlkDftThd;
	unsigned int EdgeThd;
	unsigned int WhtDftSlope;
	unsigned int BlkDftSlope;
	unsigned int EdgeGain;
	unsigned int DeNoiseEn;
	unsigned int YInitial;
	unsigned int YSlope;
	unsigned int RB_ACGain;
	unsigned int G_ACGain;
	unsigned int B_Gain1;
	unsigned int B_Gain2;
	unsigned int B_Gain3;
	unsigned int B_Slope;
	unsigned int Edge_BackFillEn;
	unsigned int G_Gain1;
	unsigned int G_Gain2;
	unsigned int G_Gain3;
	unsigned int G_Slope;
	unsigned int Flat_BackFillEn;
	unsigned int R_Gain1;
	unsigned int R_Gain2;
	unsigned int R_Gain3;
	unsigned int R_Slope;
	unsigned int Mode;
};

// cmd = 37
struct color_interpolation
{
	unsigned int Interp_Thd;
	unsigned int HVDiff;
	unsigned int LRDiff;
};

// cmd = 38
struct edge_enhance
{
	unsigned int Mode;
	unsigned int HGain;
	unsigned int VGain;
	unsigned int Thd;
	unsigned int Grads_LB;
	unsigned int Strl;
	unsigned int Slp;
	unsigned int HB;
	unsigned int LB;
};

// cmd = 39
struct de_aliasing
{
	unsigned int Enable;
	unsigned int Mode0;
	unsigned int Mode1;
	unsigned int Level;
};

// cmd = 40
struct gbgr_balance
{
	unsigned int GbGr_Enable;
	unsigned int ThdL;
	unsigned int ThdH;
	unsigned int ThdSlp;
	unsigned int WESlp;
	unsigned int WFSlp;
	unsigned int WSlpGain;
};

// cmd = 13
struct yuv1_finescaler
{
	unsigned int Scaler1_RATIO_H;
	unsigned int Scaler1_RATIO_V;
};

// cmd = 14
struct yuv1_low_pass_filter
{
	unsigned int LPF1_LPFEn;
	unsigned int LPF1_Tap1;
	unsigned int LPF1_Tap2;
	unsigned int LPF1_Tap3;
};

// cmd = 15
struct yuv1_windowing
{
	unsigned int Out1_OUT_HSTART;
	unsigned int Out1_OUT_VSTART;
	unsigned int Out1_OUT_HSIZE;
	unsigned int Out1_OUT_VSIZE;
};

// cmd = 16
struct yuv1_filter
{
	unsigned int Filter1_YfilterEn;
	unsigned int Filter1_UVfilterEn;
	unsigned int Filter1_Slope;
	unsigned int Filter1_Weight;
};

// cmd = 17
struct yuv1_edge
{
	unsigned int Edge1_EdgeFilterEn;
	unsigned int Edge1_EdgeHGain;
	unsigned int Edge1_EdgeVGain;
	unsigned int Edge1_EdgeThd;
	unsigned int Edge1_EdgeStr;
	unsigned int Edge1_EdgeSlp;
};

// cmd = 18
struct yuv2_finescaler
{
	unsigned int Scaler2_RATIO_H;
	unsigned int Scaler2_RATIO_V;
};

// cmd = 19
struct yuv2_low_pass_filter
{
	unsigned int LPF2_LPFEn;
	unsigned int LPF2_Tap1;
	unsigned int LPF2_Tap2;
	unsigned int LPF2_Tap3;
};

// cmd = 20
struct yuv2_windowing
{
	unsigned int Out2_OUT_HSTART;
	unsigned int Out2_OUT_VSTART;
	unsigned int Out2_OUT_HSIZE;
	unsigned int Out2_OUT_VSIZE;
};

// cmd = 21
struct yuv2_filter
{
	unsigned int Filter2_YfilterEn;
	unsigned int Filter2_UVfilterEn;
	unsigned int Filter2_Slope;
	unsigned int Filter2_Weight;
};

// cmd = 22
struct yuv2_edge
{
	unsigned int Edge2_EdgeFilterEn;
	unsigned int Edge2_EdgeHGain;
	unsigned int Edge2_EdgeVGain;
	unsigned int Edge2_EdgeThd;
	unsigned int Edge2_EdgeStr;
	unsigned int Edge2_EdgeSlp;
};

// cmd = 23
struct yuv_windowing
{
	unsigned int hi_start;
	unsigned int vi_start;
	unsigned int hi_size;
	unsigned int vi_size;
};

// cmd = 24
struct yuv_block_finescaler
{
	unsigned int SCAL_MODE;
	unsigned int H_RATIO;
	unsigned int V_RATIO;
	unsigned int HSIZE;
	unsigned int VSIZE;
};

// cmd = 25
struct mirror_flip
{
	unsigned int MirrorFlip_Value;
};

// cmd = 5
struct ae_arg
{
	unsigned int Source;
	unsigned int Auto_Fit;
	unsigned int SIZE_X;
	unsigned int SIZE_Y;
	unsigned int START_X;
	unsigned int START_Y;
	unsigned int SKIP_X;
	unsigned int SKIP_Y;
};

// cmd = 6
struct af_arg
{
	unsigned int Auto_Fit;
	unsigned int W0_Hstart;
	unsigned int W0_Vstart;
	unsigned int W0_Hsize;
	unsigned int W0_Vsize;
	unsigned int W0_Edge_Thd;
	unsigned int W1_Hstart;
	unsigned int W1_Vstart;
	unsigned int W1_Hsize;
	unsigned int W1_Vsize;
	unsigned int W1_Edge_Thd;
	unsigned int W0_Rpt;
	unsigned int W1_Rpt;
};

// cmd = 7
struct awb_arg
{
	unsigned int Auto_Fit;
	unsigned int SIZE_X;
	unsigned int SIZE_Y;
	unsigned int START_X;
	unsigned int START_Y;
	unsigned int SKIP_X;
	unsigned int SKIP_Y;
};

// cmd = 8
struct uv_gain
{
	int gain1;
	int gain2;
	int gain3;
	int gain4;
};

// cmd = 9
struct channel_offset
{
	unsigned int Offset_R;
	unsigned int Offset_Gb;
	unsigned int Offset_Gr;
	unsigned int Offset_B;
};

// cmd = 10
struct channel_gain
{
	unsigned int Channel_Gain_R;
	unsigned int Channel_Gain_Gb;
	unsigned int Channel_Gain_Gr;
	unsigned int Channel_Gain_B;
};

// cmd = 11
struct rgb_post_gain
{
	unsigned int Post_Gain_R;
	unsigned int Post_Gain_G;
	unsigned int Post_Gain_B;
};

// cmd = 12
struct yuv_offset_gain
{
	unsigned int YGain_B;
	int YOffset_B;
};

// cmd = 41
struct gamma_arg
{
	unsigned int GammaEn;
	unsigned int OutputVal[30];
	unsigned int OutputGamma;
	unsigned int Apply_Set;
};

// cmd = 42
struct DT_Matrix
{
	unsigned int ColorCorrect[9];
	unsigned int USat;
	unsigned int VSat;
	unsigned int UHue;
	unsigned int VHue;
	unsigned int VDS_R;
	unsigned int VDS_USat;
	unsigned int VDS_VSat;
	unsigned int Reset;
	unsigned int OutputMatrix[9];
};

// cmd = 35
struct NLMTab
{
	unsigned int Load;
	unsigned int Reset;
	unsigned int Enable;
	unsigned int Area0[4];
	unsigned int Area1[4];
	unsigned int Area2[4];
	unsigned int Area3[4];
	unsigned int Area4[4];
	unsigned int Area5[4];
	unsigned int Area6[4];
	unsigned int Area7[4];
};

// cmd = 29
struct UV_Slice
{
	unsigned int Slice;
	unsigned int Slice_Gain;
	unsigned int Slice_Offset;
	unsigned int Slice_Auto;
};

// cmd = 30
struct HL_CS
{
	unsigned int H_Thd;
	unsigned int HLCSEn;
	unsigned int H_Slope;
	unsigned int Focus_R;
	unsigned int L_Thd;
	unsigned int Focus_G;
	unsigned int L_Slope;
	unsigned int Focus_B;
};

// cmd = 31
struct SUV_arg
{
	unsigned int Enable;
	unsigned int Start;
	unsigned int Slope;
};

// cmd = 32
struct Domain_Transfer
{
	unsigned int YR;
	unsigned int YG;
	unsigned int YB;
	int UR;
	int UG;
	int UB;
	int VR;
	int VG;
	int VB;
	unsigned int HB0;
	unsigned int LB1;
	unsigned int LB4;
	unsigned int HB1;
	unsigned int HB4;
	unsigned int LB2;
	unsigned int LB5;
	unsigned int HB2;
	unsigned int HB5;
	unsigned int LB3;
	unsigned int LB6;
	unsigned int HB3;
	unsigned int HB6;
	unsigned int Color_Place;
	unsigned int YGain;
	unsigned int Matrix_Index;
};