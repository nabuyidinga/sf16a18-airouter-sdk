/*
 *
 * Copyright (C) 2016 Siflower Solutions
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <linux/gpio.h>
#include <linux/module.h>


static u32 calculate_sysclk_rate(u32 param_rate)
{
	u32 sysclk;
	switch(param_rate){
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 96000:
		sysclk = 12288000;
		break;
	case 8018:
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		sysclk = 11289600;
		break;
	default:
		printk(KERN_ERR"PCM: unsupport sample rate!\n");
		sysclk = 0;
	}
	return sysclk;
}

static int sfax8_hw_params_pcm(struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct device *dev = rtd->card->dev;
	int err;
	u32 param_rate, sysclk;
	u32 codec_fmt, cpu_fmt;
	param_rate = params_rate(params);

	sysclk = calculate_sysclk_rate(param_rate);
	if(!sysclk)
		return -EINVAL;

	codec_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED;
	cpu_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_IB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED;

	err = snd_soc_dai_set_fmt(codec_dai, codec_fmt);
	if (err < 0){
		dev_err(dev,"%s: ERROR: snd_soc_dai_set_fmt set codec dai error %d!\n", __func__, err);
	}

	err = snd_soc_dai_set_fmt(cpu_dai, cpu_fmt);
	if (err < 0){
		dev_err(dev,"%s: ERROR: snd_soc_dai_set_fmt set cpu dai error %d!\n", __func__, err);
	}

	/* Set the codec system clock for DAC and ADC */
	err =
	    snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, SND_SOC_CLOCK_IN);

	if (err < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return err;
	}


	err = snd_soc_dai_set_sysclk(cpu_dai, 1, sysclk, SND_SOC_CLOCK_OUT);
	if (err < 0) {
		printk(KERN_ERR "can't set cpu dai system clock\n");
		return err;
	}
	return 0;
}

static int sfax8_hw_params_i2s(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
#if  !defined(CONFIG_SND_SOC_ES7149_MODULE) && !defined(CONFIG_SND_SOC_ES7149_MODULE)
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
#endif
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	//struct device *dev = rtd->card->dev;
	int err;
	u32 param_rate, sysclk;
	u32 codec_fmt, cpu_fmt;
	param_rate = params_rate(params);
	sysclk = calculate_sysclk_rate(param_rate);
	if(!sysclk)
		return -EINVAL;

	codec_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED;
	cpu_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_IB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED;

#if  !defined(CONFIG_SND_SOC_ES7149_MODULE) && !defined(CONFIG_SND_SOC_ES7149_MODULE)
	err = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, SND_SOC_CLOCK_IN);
	if (err < 0) {
		printk(KERN_ERR "can't set codec dai system clock\n");
		return err;
	}
#endif

	err = snd_soc_dai_set_sysclk(cpu_dai, 0, sysclk, SND_SOC_CLOCK_OUT);
	if (err < 0) {
		printk(KERN_ERR "can't set cpu dai system clock\n");
		return err;
	}

	return 0;
}


static struct snd_soc_ops sfax8_ops[] = {
	{
	.hw_params = sfax8_hw_params_pcm,
	},

	{
	.hw_params = sfax8_hw_params_i2s,
	},

};

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link sfax8_dai[] = {

#if defined(CONFIG_SND_SOC_SFAX8_I2S) || defined(CONFIG_SND_SOC_SFAX8_I2S_MODULE)

#if defined(CONFIG_SND_SOC_ES8388S) || defined(CONFIG_SND_SOC_ES8388S_MODULE)
	{
	.name = "SFA18-audio3",
	.stream_name = "ES8388-i2s0",
	.cpu_dai_name = "18000000.i2s",
	.codec_dai_name = "es8388s-hifi",
	.platform_name = "18000000.i2s",
	.codec_name = "es8388s-codec.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS ,
	.ops = &sfax8_ops,
	},
#endif //ES8388 end

#if defined(CONFIG_SND_SOC_ES8316_MODULE) || defined(CONFIG_SND_SOC_ES8316)
	{
	.name = "SFA18-audio4",
	.stream_name = "ES8316-i2s0",
	.cpu_dai_name = "18000000.i2s",
	.codec_dai_name = "ES8316 HiFi",
	.platform_name = "18000000.i2s",
	.codec_name = "es8316.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBM_CFM ,
	.ops = &sfax8_ops[1],
	},

	{
	.name = "SFA18-audio5",
	.stream_name = "ES8316-i2s1",
	.cpu_dai_name = "18001000.i2s",
	.codec_dai_name = "ES8316 HiFi",
	.platform_name = "18001000.i2s",
	.codec_name = "es8316.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS ,
	.ops = &sfax8_ops[1],
	},
#endif //ES8316 end

#if  defined(CONFIG_SND_SOC_ES7149_MODULE) || defined(CONFIG_SND_SOC_ES7149_MODULE)
	{
	.name = "SFA18-audio6",
	.stream_name = "ES7149-i2s0",
	.cpu_dai_name = "18000000.i2s",
	.codec_dai_name = "dai_es7149",
	.platform_name = "18000000.i2s",
	.codec_name = "codec-es7149@0",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBM_CFM ,
	.ops = &sfax8_ops[1],
	},
#if  !defined(CONFIG_SND_SOC_ES7210) && !defined(CONFIG_SND_SOC_ES7210_MODULE)
	{
	.name = "SFA18-audio7",
	.stream_name = "ES7149-i2s1",
	.cpu_dai_name = "18001000.i2s",
	.codec_dai_name = "dai_es7149",
	.platform_name = "18001000.i2s",
	.codec_name = "codec-es7149@0",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS ,
	.ops = &sfax8_ops[1],
	},
#endif
#endif //ES7149 end

#if defined(CONFIG_SND_SOC_RT5631) || defined(CONFIG_SND_SOC_RT5631_MODULE)
	{
	.name = "SFA18-audio8",
	.stream_name = "ES8316-i2s0",
	.cpu_dai_name = "18000000.i2s",
	.codec_dai_name = "rt5631-hifi",
	.platform_name = "18000000.i2s",
	.codec_name = "rt5631.1-001a",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS ,
	.ops = &sfax8_ops[1],
	},
#endif //RN5631 end

#if defined(CONFIG_SND_SOC_AD82584F) || defined(CONFIG_SND_SOC_AD82584F_MODULE)
	{
	.name = "SFA18-audio8",
	.stream_name = "ad82584f-i2s0",
	.cpu_dai_name = "18000000.i2s",
	.codec_dai_name = "ad82584f",
	.platform_name = "18000000.i2s",
	.codec_name = "ad82584f.1-0030",
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS ,
	.ops = &sfax8_ops[1],
	},

#endif

#endif //I2S end

#if defined(CONFIG_SND_SOC_SFAX8_PCM) || defined(CONFIG_SND_SOC_SFAX8_PCM_MODULE)

#if defined(CONFIG_SND_SOC_ES8388S) || defined(CONFIG_SND_SOC_ES8388S_MODULE)
	{
	.name = "SFA18-audio0",
	.stream_name = "ES8388-pcm0",
	.cpu_dai_name = "18400000.pcm",
	.codec_dai_name = "es8388s-hifi",
	.platform_name = "18400000.pcm",
	.codec_name = "es8388s-codec.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	.ops = &sfax8_ops,
	},
/*	{
	.name = "SFA18-audio1",
	.stream_name = "ES8388-pcm1",
	.cpu_dai_name = "18401000.pcm",
	.codec_dai_name = "es8388s-hifi",
	.platform_name = "18401000.pcm",
	.codec_name = "es8388s-codec.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	.ops = &sfax8_ops,
	},
*/
#endif // ES8388 end

#if defined(CONFIG_SND_SOC_ES8316_MODULE) || defined(CONFIG_SND_SOC_ES8316)
	{
	.name = "SFA18-audio0",
	.stream_name = "ES8316-pcm0",
	.cpu_dai_name = "18400000.pcm",
	.codec_dai_name = "ES8316 HiFi",
	.platform_name = "18400000.pcm",
	.codec_name = "es8316.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	.ops = &sfax8_ops[0],
	},
	{
	.name = "SFA18-audio1",
	.stream_name = "ES8316-pcm1",
	.cpu_dai_name = "18401000.pcm",
	.codec_dai_name = "ES8316 HIFI",
	.platform_name = "18401000.pcm",
	.codec_name = "es8316.1-0011",
	.dai_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	.ops = &sfax8_ops[0],
	},
#endif //ES8316 end

#if  defined(CONFIG_SND_SOC_ES7210) || defined(CONFIG_SND_SOC_ES7210_MODULE)
	{
	.name = "SFA18-audio2",
	.stream_name = "ES7210-pcm0",
	.cpu_dai_name = "18400000.pcm",
	.codec_dai_name = "ES7210 4CH ADC 0",
	.platform_name = "18400000.pcm",
#if defined(CONFIG_DT_SF16A18_FULLMASK_AIR001_V4) || defined(CONFIG_DT_SF16A18_FULLMASK_AIR001_V5)
	.codec_name = "es7210.0-0040",
#else
	.codec_name = "es7210.2-0040",
#endif
	.dai_fmt = SND_SOC_DAIFMT_DSP_B | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	.ops = &sfax8_ops[0],
	},
#endif //es7210 end

#endif //PCM end



};

/* Audio machine driver */
static struct snd_soc_card snd_soc_card_sfax8 = {
	.name = "SFAX8",
	.owner = THIS_MODULE,
	.dai_link = sfax8_dai,
	.num_links = ARRAY_SIZE(sfax8_dai),
};

static struct platform_device *sfax8_snd_device;

static int __init sfax8_soc_init(void)
{
	int err;

	sfax8_snd_device = platform_device_alloc("soc-audio", -1);
	if (!sfax8_snd_device)
		return -ENOMEM;

	platform_set_drvdata(sfax8_snd_device, &snd_soc_card_sfax8);
	err = platform_device_add(sfax8_snd_device);
	if (err)
		goto err1;

	return 0;

err1:
	platform_device_put(sfax8_snd_device);

	return err;

}

static void __exit sfax8_soc_exit(void)
{
	platform_device_unregister(sfax8_snd_device);
}

module_init(sfax8_soc_init);
module_exit(sfax8_soc_exit);

MODULE_AUTHOR("Xijun Guo <xijun.guo@siflower.com.cn>");
MODULE_DESCRIPTION("ALSA SoC SFA18");
MODULE_LICENSE("GPL");
