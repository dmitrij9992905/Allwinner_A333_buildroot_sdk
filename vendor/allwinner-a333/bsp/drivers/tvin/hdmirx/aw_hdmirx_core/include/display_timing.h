/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2024 - 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs hdmirx driver.
 *
 * Copyright (C) 2024 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef __DISPLAY_TIMING_H
#define __DISPLAY_TIMING_H


/* Version Definition */

/* Attribute Definition */
#define TFD_VERSION                                        0x17218018
#define TFD_TSE_DATA_ENDIAN_TYPE                           0x00000001
#define AT_SINGLE                                          0x0000
#define AI_SINGLE_DEFAULT                                  0x00000000

#define AT_SIGNAL_CHANNEL                                  0x0001
#define AI_SIGNAL_CHANNEL_MPEG1                            0x00010000
#define AI_SIGNAL_CHANNEL_NONE                             0x00010001
#define AI_SIGNAL_CHANNEL_HDTV1                            0x00010002
#define AI_SIGNAL_CHANNEL_HDTV2                            0x00010003
#define AI_SIGNAL_CHANNEL_PC                               0x00010004
#define AI_SIGNAL_CHANNEL_TV                               0x00010005
#define AI_SIGNAL_CHANNEL_CVBS1                            0x00010006
#define AI_SIGNAL_CHANNEL_CVBS2                            0x00010007
#define AI_SIGNAL_CHANNEL_CVBS3                            0x00010008
#define AI_SIGNAL_CHANNEL_SVIDEO1                          0x00010009
#define AI_SIGNAL_CHANNEL_SVIDEO2                          0x0001000A
#define AI_SIGNAL_CHANNEL_SVIDEO3                          0x0001000B
#define AI_SIGNAL_CHANNEL_YCBCR1                           0x0001000C
#define AI_SIGNAL_CHANNEL_YCBCR2                           0x0001000D
#define AI_SIGNAL_CHANNEL_MPEG2                            0x00010016
#define AI_SIGNAL_CHANNEL_SCART                            0x0001000F
#define AI_SIGNAL_CHANNEL_SCART2                           0x00010010
#define AI_SIGNAL_CHANNEL_HDMI1                            0x00010011
#define AI_SIGNAL_CHANNEL_HDMI2                            0x00010013
#define AI_SIGNAL_CHANNEL_HDMI3                            0x00010014
#define AI_SIGNAL_CHANNEL_HDMI4                            0x00010015
#define AI_SIGNAL_CHANNEL_STILLIMAGE                       0x00010012
#define AI_SIGNAL_CHANNEL_WISELINK                         0x00010017
#define AG_SIGNAL_CHANNEL_TCD                              0x00018000
#define AG_SIGNAL_CHANNEL_CVBS                             0x00018001
#define AG_SIGNAL_CHANNEL_SVIDEO                           0x00018002
#define AG_SIGNAL_CHANNEL_YCBCR                            0x00018003
#define AG_SIGNAL_CHANNEL_SCART1                           0x00018004
#define AG_SIGNAL_CHANNEL_HDTV                             0x00018005
#define AG_SIGNAL_CHANNEL_ALL_CH                           0x00018006
#define AG_SIGNAL_CHANNEL_HDMI                             0x00018007
#define AG_SIGNAL_CHANNEL_MPEG                             0x00018008

#define AT_SIGNAL_MODE                                     0x0002
#define AI_SIGNAL_MODE_1080P                               0x00020000
#define AI_SIGNAL_MODE_UNKNOW                              0x00020002
#define AI_SIGNAL_MODE_NO_SIGNAL                           0x00020003
#define AI_SIGNAL_MODE_480I                                0x0002000B
#define AI_SIGNAL_MODE_480IX2                              0x0002000C
#define AI_SIGNAL_MODE_480P                                0x0002000D
#define AI_SIGNAL_MODE_480PX2                              0x0002000E
#define AI_SIGNAL_MODE_576I                                0x0002000F
#define AI_SIGNAL_MODE_576IX2                              0x00020010
#define AI_SIGNAL_MODE_576P                                0x00020011
#define AI_SIGNAL_MODE_576PX2                              0x00020012
#define AI_SIGNAL_MODE_720P                                0x00020015
#define AI_SIGNAL_MODE_1080I                               0x00020016
#define AI_SIGNAL_MODE_1440P2560                           0x00020095
#define AI_SIGNAL_MODE_2160P1920                           0x00020094
#define AI_SIGNAL_MODE_2160P3840                           0x0002008F
#define AI_SIGNAL_MODE_2160P4096                           0x00020093
#define AI_SIGNAL_MODE_640_350                             0x00020042
#define AI_SIGNAL_MODE_640_400                             0x00020043
#define AI_SIGNAL_MODE_640_480                             0x00020044
#define AI_SIGNAL_MODE_720_400                             0x00020045
#define AI_SIGNAL_MODE_720_576                             0x00020046
#define AI_SIGNAL_MODE_800_600                             0x00020047
#define AI_SIGNAL_MODE_832_624                             0x00020048
#define AI_SIGNAL_MODE_848_480                             0x00020049
#define AI_SIGNAL_MODE_852_480                             0x0002004A
#define AI_SIGNAL_MODE_960_600                             0x0002004B
#define AI_SIGNAL_MODE_1024_768                            0x0002004C
#define AI_SIGNAL_MODE_1152_864                            0x0002004D
#define AI_SIGNAL_MODE_1152_870                            0x0002004E
#define AI_SIGNAL_MODE_1152_900                            0x0002004F
#define AI_SIGNAL_MODE_1280_720                            0x00020050
#define AI_SIGNAL_MODE_1280_768                            0x00020051
#define AI_SIGNAL_MODE_1280_800                            0x00020052
#define AI_SIGNAL_MODE_1280_960                            0x00020053
#define AI_SIGNAL_MODE_1280_1024                           0x00020054
#define AI_SIGNAL_MODE_1360_765                            0x0002008A
#define AI_SIGNAL_MODE_1360_768                            0x00020055
#define AI_SIGNAL_MODE_1600_1200                           0x00020056
#define AI_SIGNAL_MODE_1920_1080                           0x00020057
#define AI_SIGNAL_MODE_1920_1200                           0x0002008D
#define AI_SIGNAL_MODE_1365_1024                           0x00020058
#define AI_SIGNAL_MODE_1366_768                            0x00020059
#define AI_SIGNAL_MODE_1440_900                            0x0002005A
#define AI_SIGNAL_MODE_1400_1050                           0x0002008E
#define AI_SIGNAL_MODE_1680_1050                           0x0002005B
#define AI_SIGNAL_MODE_1792_1344                           0x0002005C
#define AI_SIGNAL_MODE_1856_1392                           0x0002005D
#define AI_SIGNAL_MODE_2560_1440                           0x00020096
#define AI_SIGNAL_MODE_3840_2160                           0x00020091
#define AI_SIGNAL_MODE_4096_2160                           0x00020092
#define AI_SIGNAL_MODE_720_480I                            0x00020086
#define AI_SIGNAL_MODE_720_240P                            0x00020087
#define AI_SIGNAL_MODE_720_288P                            0x00020089
#define AI_SIGNAL_MODE_640_480P                            0x00020088
#define AI_SIGNAL_MODE_240PX2                              0x0002008B
#define AI_SIGNAL_MODE_288PX2                              0x0002008C

#define AT_SIGNAL_FRAMERATE                                0x0003
#define AI_SIGNAL_FRAMERATE_60                             0x00030000
#define AI_SIGNAL_FRAMERATE_23_97                          0x0003000B
#define AI_SIGNAL_FRAMERATE_24                             0x00030001
#define AI_SIGNAL_FRAMERATE_25                             0x00030002
#define AI_SIGNAL_FRAMERATE_29_97                          0x00030003
#define AI_SIGNAL_FRAMERATE_30                             0x00030004
#define AI_SIGNAL_FRAMERATE_43                             0x00030005
#define AI_SIGNAL_FRAMERATE_47_952                         0x0003001A
#define AI_SIGNAL_FRAMERATE_48                             0x00030006
#define AI_SIGNAL_FRAMERATE_50                             0x00030007
#define AI_SIGNAL_FRAMERATE_56                             0x00030008
#define AI_SIGNAL_FRAMERATE_59_934                         0x00030009
#define AI_SIGNAL_FRAMERATE_59_94                          0x0003000A
#define AI_SIGNAL_FRAMERATE_67                             0x0003000C
#define AI_SIGNAL_FRAMERATE_70                             0x0003000D
#define AI_SIGNAL_FRAMERATE_72                             0x0003000E
#define AI_SIGNAL_FRAMERATE_75                             0x0003000F
#define AI_SIGNAL_FRAMERATE_82                             0x00030010
#define AI_SIGNAL_FRAMERATE_85                             0x00030011
#define AI_SIGNAL_FRAMERATE_90                             0x00030012
#define AI_SIGNAL_FRAMERATE_100                            0x00030013
#define AI_SIGNAL_FRAMERATE_119_88                         0x00030019
#define AI_SIGNAL_FRAMERATE_120                            0x00030014
#define AG_SIGNAL_FRAMERATE_ALLEXCEPT120                   0x00038003
#define AG_SIGNAL_FRAMERATE_PC                             0x00038000
#define AG_SIGNAL_FRAMERATE_VIDEO                          0x00038001
#define AI_SIGNAL_FRAMERATE_8                              0x00030015
#define AI_SIGNAL_FRAMERATE_12                             0x00030016
#define AI_SIGNAL_FRAMERATE_15                             0x00030017
#define AI_SIGNAL_FRAMERATE_16                             0x00030018
#define AG_SIGNAL_FRAMERATE_LOWFRAMERATE                   0x00038002
#define AG_SIGNAL_FRAMERATE_HIGHFRAMERATE                  0x00038004
#define AG_SIGNAL_FRAMERATE_FRAMERATEOVER30HZ              0x00038005

#define AT_SIGNAL_FORMAT                                   0x001D
#define AI_SIGNAL_FORMAT_YUV422_101010                     0x001D0000
#define AI_SIGNAL_FORMAT_RGB_888                           0x001D0007
#define AI_SIGNAL_FORMAT_RGB_101010                        0x001D0001
#define AI_SIGNAL_FORMAT_RGB_121212                        0x001D0002
#define AI_SIGNAL_FORMAT_YUV444_888                        0x001D0003
#define AI_SIGNAL_FORMAT_YUV444_101010                     0x001D0004
#define AI_SIGNAL_FORMAT_YUV444_121212                     0x001D0005
#define AI_SIGNAL_FORMAT_YUV422_888                        0x001D0006
#define AI_SIGNAL_FORMAT_YUV422_121212                     0x001D0008
#define AI_SIGNAL_FORMAT_YUV420_888                        0x001D0009
#define AI_SIGNAL_FORMAT_YUV420_101010                     0x001D000A
#define AI_SIGNAL_FORMAT_YUV420_121212                     0x001D000B
#define AG_SIGNAL_FORMAT_RGB                               0x001D8000
#define AG_SIGNAL_FORMAT_YUV444                            0x001D8001
#define AG_SIGNAL_FORMAT_YUV422                            0x001D8002
#define AG_SIGNAL_FORMAT_YUV420                            0x001D8003
#define AG_SIGNAL_FORMAT_YUV                               0x001D8004
#define AG_SIGNAL_FORMAT_ALL_444                           0x001D8005
#define AG_SIGNAL_FORMAT_ALL_FORMAT                        0x001D8006

#define AT_SIGNAL_RANGE                                    0x003A
#define AI_SIGNAL_RANGE_LIMIT_RANGE                        0x003A0000
#define AI_SIGNAL_RANGE_FULL_RANGE                         0x003A0001

#define AT_SIGNAL_COLORSPACE                               0x0044
#define AI_SIGNAL_COLORSPACE_BT709                         0x00440000
#define AI_SIGNAL_COLORSPACE_BT601                         0x00440001
#define AI_SIGNAL_COLORSPACE_BT2020_NCLYCC                 0x00440002
#define AI_SIGNAL_COLORSPACE_BT2020_CLYCC                  0x00440009
#define AI_SIGNAL_COLORSPACE_XVYCC                         0x00440003
#define AI_SIGNAL_COLORSPACE_RGB                           0x00440004
#define AG_SIGNAL_COLORSPACE_ALL_SIGNAL_CS                 0x00448001

#define AT_ENHANCEDVIDEOMODE                               0x003F
#define AI_ENHANCEDVIDEOMODE_SDR                           0x003F0000
#define AI_ENHANCEDVIDEOMODE_EDR                           0x003F0001
#define AI_ENHANCEDVIDEOMODE_HDR10                         0x003F0002
#define AI_ENHANCEDVIDEOMODE_HDR10_SGMIP                   0x003F0003
#define AI_ENHANCEDVIDEOMODE_HDR10PLUS                     0x003F0005
#define AI_ENHANCEDVIDEOMODE_HLG                           0x003F0004
#define AI_ENHANCEDVIDEOMODE_EDR_LL                        0x003F0006
#define AI_ENHANCEDVIDEOMODE_HDR_PSL                       0x003F0007
#define AI_ENHANCEDVIDEOMODE_HDR_ITMO                      0x003F0008
#define AG_ENHANCEDVIDEOMODE_ALL_MODE                      0x003F8000
#define AG_ENHANCEDVIDEOMODE_NORMAL_MODE                   0x003F8001

#define AT_HDMIRX_OUTPUT_CHANNEL                           0x005E
#define AI_HDMIRX_OUTPUT_CHANNEL_MAIN                      0x005E0000
#define AI_HDMIRX_OUTPUT_CHANNEL_SUB                       0x005E0001

#endif
