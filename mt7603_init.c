/*
 * Copyright (C) 2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "mt7603.h"

struct mt7603_dev *mt7603_alloc_device(struct device *pdev)
{
	struct ieee80211_hw *hw;
	struct mt7603_dev *dev;

	hw = ieee80211_alloc_hw(sizeof(*dev), &mt7603_ops);
	if (!hw)
		return NULL;

	dev = hw->priv;
	dev->mt76.dev = pdev;
	dev->mt76.hw = hw;

	return dev;
}

int mt7603_mac_reset(struct mt7603_dev *dev)
{
	u32 val;

	val = mt76_rr(dev, MT_WPDMA_GLO_CFG);

	val &= ~(MT_WPDMA_GLO_CFG_TX_DMA_EN |
		 MT_WPDMA_GLO_CFG_TX_DMA_BUSY |
		 MT_WPDMA_GLO_CFG_RX_DMA_EN |
		 MT_WPDMA_GLO_CFG_RX_DMA_BUSY |
		 MT_WPDMA_GLO_CFG_DMA_BURST_SIZE);
	val |= MT76_SET(MT_WPDMA_GLO_CFG_DMA_BURST_SIZE, 3);

	mt76_wr(dev, MT_WPDMA_GLO_CFG, val);
	return 0;
}

int mt7603_mac_start(struct mt7603_dev *dev)
{
	mt76_set(dev, MT_WPDMA_GLO_CFG,
		 MT_WPDMA_GLO_CFG_TX_DMA_EN |
		 MT_WPDMA_GLO_CFG_RX_DMA_EN);

	mt76_clear(dev, MT_WPDMA_GLO_CFG, MT_WPDMA_GLO_CFG_TX_WRITEBACK_DONE);
	return 0;
}

int mt7603_init_hardware(struct mt7603_dev *dev)
{
	int ret;

	ret = mt7603_mac_reset(dev);
	if (ret)
		return ret;

	ret = mt7603_dma_init(dev);
	if (ret)
		return ret;

	set_bit(MT76_STATE_INITIALIZED, &dev->mt76.state);
	mt7603_irq_enable(dev, MT_INT_RX_DONE_ALL | MT_INT_TX_DONE_ALL);

	ret = mt7603_mac_start(dev);
	if (ret)
		return ret;

	ret = mt7603_mcu_init(dev);
	if (ret)
		return ret;

	return 0;
}
