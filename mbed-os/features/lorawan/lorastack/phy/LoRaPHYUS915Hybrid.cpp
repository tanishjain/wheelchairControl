/**
 *  @file LoRaPHYUS915Hybrid.cpp
 *
 *  @brief Implements LoRaPHY for US 915 MHz Hybrid band
 *
 *  \code
 *   ______                              _
 *  / _____)             _              | |
 * ( (____  _____ ____ _| |_ _____  ____| |__
 *  \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *  _____) ) ____| | | || |_| ____( (___| | | |
 * (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *   (C)2013 Semtech
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions===============
 *
 * \endcode
 *
 *
 * License: Revised BSD License, see LICENSE.TXT file include in the project
 *
 * Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
 *
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "LoRaPHYUS915Hybrid.h"
#include "lora_phy_ds.h"

/*!
 * Minimal datarate that can be used by the node
 */
#define US915_HYBRID_TX_MIN_DATARATE                DR_0

/*!
 * Maximal datarate that can be used by the node
 */
#define US915_HYBRID_TX_MAX_DATARATE                DR_4

/*!
 * Minimal datarate that can be used by the node
 */
#define US915_HYBRID_RX_MIN_DATARATE                DR_8

/*!
 * Maximal datarate that can be used by the node
 */
#define US915_HYBRID_RX_MAX_DATARATE                DR_13

/*!
 * Default datarate used by the node
 */
#define US915_HYBRID_DEFAULT_DATARATE               DR_0

/*!
 * Minimal Rx1 receive datarate offset
 */
#define US915_HYBRID_MIN_RX1_DR_OFFSET              0

/*!
 * Maximal Rx1 receive datarate offset
 */
#define US915_HYBRID_MAX_RX1_DR_OFFSET              3

/*!
 * Default Rx1 receive datarate offset
 */
#define US915_HYBRID_DEFAULT_RX1_DR_OFFSET          0

/*!
 * Minimal Tx output power that can be used by the node
 */
#define US915_HYBRID_MIN_TX_POWER                   TX_POWER_10

/*!
 * Maximal Tx output power that can be used by the node
 */
#define US915_HYBRID_MAX_TX_POWER                   TX_POWER_0

/*!
 * Default Tx output power used by the node
 */
#define US915_HYBRID_DEFAULT_TX_POWER               TX_POWER_0

/*!
 * Default Max ERP
 */
#define US915_HYBRID_DEFAULT_MAX_ERP                30.0f

/*!
 * ADR Ack limit
 */
#define US915_HYBRID_ADR_ACK_LIMIT                  64

/*!
 * ADR Ack delay
 */
#define US915_HYBRID_ADR_ACK_DELAY                  32

/*!
 * Enabled or disabled the duty cycle
 */
#define US915_HYBRID_DUTY_CYCLE_ENABLED             0

/*!
 * Maximum RX window duration
 */
#define US915_HYBRID_MAX_RX_WINDOW                  3000

/*!
 * Receive delay 1
 */
#define US915_HYBRID_RECEIVE_DELAY1                 1000

/*!
 * Receive delay 2
 */
#define US915_HYBRID_RECEIVE_DELAY2                 2000

/*!
 * Join accept delay 1
 */
#define US915_HYBRID_JOIN_ACCEPT_DELAY1             5000

/*!
 * Join accept delay 2
 */
#define US915_HYBRID_JOIN_ACCEPT_DELAY2             6000

/*!
 * Maximum frame counter gap
 */
#define US915_HYBRID_MAX_FCNT_GAP                   16384

/*!
 * Ack timeout
 */
#define US915_HYBRID_ACKTIMEOUT                     2000

/*!
 * Random ack timeout limits
 */
#define US915_HYBRID_ACK_TIMEOUT_RND                1000

/*!
 * Second reception window channel frequency definition.
 */
#define US915_HYBRID_RX_WND_2_FREQ                  923300000

/*!
 * Second reception window channel datarate definition.
 */
#define US915_HYBRID_RX_WND_2_DR                    DR_8

/*!
 * Band 0 definition
 * { DutyCycle, TxMaxPower, LastJoinTxDoneTime, LastTxDoneTime, TimeOff }
 */
static const band_t US915_HYBRID_BAND0 = { 1, US915_HYBRID_MAX_TX_POWER, 0, 0, 0 }; //  100.0 %

/*!
 * Defines the first channel for RX window 1 for US band
 */
#define US915_HYBRID_FIRST_RX1_CHANNEL              ( (uint32_t) 923300000 )

/*!
 * Defines the last channel for RX window 1 for US band
 */
#define US915_HYBRID_LAST_RX1_CHANNEL               ( (uint32_t) 927500000 )

/*!
 * Defines the step width of the channels for RX window 1
 */
#define US915_HYBRID_STEPWIDTH_RX1_CHANNEL          ( (uint32_t) 600000 )

/*!
 * Data rates table definition
 */
static const uint8_t datarates_US915_HYBRID[]  = { 10, 9, 8,  7,  8,  0,  0, 0, 12, 11, 10, 9, 8, 7, 0, 0 };

/*!
 * Bandwidths table definition in Hz
 */
static const uint32_t bandwidths_US915_HYBRID[] = { 125000, 125000, 125000, 125000, 500000, 0, 0, 0, 500000, 500000, 500000, 500000, 500000, 500000, 0, 0 };

/*!
 * Up/Down link data rates offset definition
 */
static const int8_t datarate_offsets_US915_HYBRID[5][4] =
{
    { DR_10, DR_9 , DR_8 , DR_8  }, // DR_0
    { DR_11, DR_10, DR_9 , DR_8  }, // DR_1
    { DR_12, DR_11, DR_10, DR_9  }, // DR_2
    { DR_13, DR_12, DR_11, DR_10 }, // DR_3
    { DR_13, DR_13, DR_12, DR_11 }, // DR_4
};

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
static const uint8_t max_payloads_US915_HYBRID[] = { 11, 53, 125, 242, 242, 0, 0, 0, 53, 129, 242, 242, 242, 242, 0, 0 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
static const uint8_t max_payloads_with_repeater_US915_HYBRID[] = { 11, 53, 125, 242, 242, 0, 0, 0, 33, 109, 222, 222, 222, 222, 0, 0 };

LoRaPHYUS915Hybrid::LoRaPHYUS915Hybrid(LoRaWANTimeHandler &lora_time)
    : LoRaPHY(lora_time)
{
    bands[0] = US915_HYBRID_BAND0;

    // Channels
    // 125 kHz channels
    for (uint8_t i = 0; i < US915_HYBRID_MAX_NB_CHANNELS - 8; i++) {
        channels[i].frequency = 902300000 + i * 200000;
        channels[i].dr_range.value = ( DR_3 << 4 ) | DR_0;
        channels[i].band = 0;
    }

    // 500 kHz channels
    for (uint8_t i = US915_HYBRID_MAX_NB_CHANNELS - 8; i < US915_HYBRID_MAX_NB_CHANNELS; i++) {
        channels[i].frequency = 903000000 + (i - (US915_HYBRID_MAX_NB_CHANNELS - 8)) * 1600000;
        channels[i].dr_range.value = ( DR_4 << 4 ) | DR_4;
        channels[i].band = 0;
    }

    // ChannelsMask
    default_channel_mask[0] = 0x00FF;
    default_channel_mask[1] = 0x0000;
    default_channel_mask[2] = 0x0000;
    default_channel_mask[3] = 0x0000;
    default_channel_mask[4] = 0x0001;

    memset(channel_mask, 0, sizeof(channel_mask));
    memset(current_channel_mask, 0, sizeof(current_channel_mask));

    // Copy channels default mask
    copy_channel_mask(channel_mask, default_channel_mask, US915_HYBRID_CHANNEL_MASK_SIZE);

    // Copy into channels mask remaining
    copy_channel_mask(current_channel_mask, channel_mask, US915_HYBRID_CHANNEL_MASK_SIZE);

    // set default channels
    phy_params.channels.channel_list = channels;
    phy_params.channels.channel_list_size = US915_HYBRID_MAX_NB_CHANNELS;
    phy_params.channels.mask = channel_mask;
    phy_params.channels.default_mask = default_channel_mask;
    phy_params.channels.mask_size = US915_HYBRID_CHANNEL_MASK_SIZE;

    // set bands for US915_HYBRID spectrum
    phy_params.bands.table = (void *) bands;
    phy_params.bands.size = US915_HYBRID_MAX_NB_BANDS;

    // set bandwidths available in US915_HYBRID spectrum
    phy_params.bandwidths.table = (void *) bandwidths_US915_HYBRID;
    phy_params.bandwidths.size = 16;

    // set data rates available in US915_HYBRID spectrum
    phy_params.datarates.table = (void *) datarates_US915_HYBRID;
    phy_params.datarates.size = 16;

    // set payload sizes with respect to data rates
    phy_params.payloads.table = (void *) max_payloads_US915_HYBRID;
    phy_params.payloads.size = 16;
    phy_params.payloads_with_repeater.table = (void *) max_payloads_with_repeater_US915_HYBRID;
    phy_params.payloads_with_repeater.size = 16;

    // dwell time setting
    phy_params.ul_dwell_time_setting = 0;
    phy_params.dl_dwell_time_setting = 0;

    // set initial and default parameters
    phy_params.duty_cycle_enabled = US915_HYBRID_DUTY_CYCLE_ENABLED;
    phy_params.accept_tx_param_setup_req = false;
    phy_params.fsk_supported = false;
    phy_params.cflist_supported = false;
    phy_params.dl_channel_req_supported = false;
    phy_params.custom_channelplans_supported = false;
    phy_params.default_channel_cnt = US915_HYBRID_MAX_NB_CHANNELS;
    phy_params.max_channel_cnt = US915_HYBRID_MAX_NB_CHANNELS;
    phy_params.cflist_channel_cnt = 0;
    phy_params.min_tx_datarate = US915_HYBRID_TX_MIN_DATARATE;
    phy_params.max_tx_datarate = US915_HYBRID_TX_MAX_DATARATE;
    phy_params.min_rx_datarate = US915_HYBRID_RX_MIN_DATARATE;
    phy_params.max_rx_datarate = US915_HYBRID_RX_MAX_DATARATE;
    phy_params.default_datarate = US915_HYBRID_DEFAULT_DATARATE;
    phy_params.default_max_datarate = US915_HYBRID_TX_MAX_DATARATE;
    phy_params.min_rx1_dr_offset = US915_HYBRID_MIN_RX1_DR_OFFSET;
    phy_params.max_rx1_dr_offset = US915_HYBRID_MAX_RX1_DR_OFFSET;
    phy_params.default_rx1_dr_offset = US915_HYBRID_DEFAULT_RX1_DR_OFFSET;
    phy_params.min_tx_power = US915_HYBRID_MIN_TX_POWER;
    phy_params.max_tx_power = US915_HYBRID_MAX_TX_POWER;
    phy_params.default_tx_power = US915_HYBRID_DEFAULT_TX_POWER;
    phy_params.default_max_eirp = 0;
    phy_params.default_antenna_gain = 0;
    phy_params.adr_ack_limit = US915_HYBRID_ADR_ACK_LIMIT;
    phy_params.adr_ack_delay = US915_HYBRID_ADR_ACK_DELAY;
    phy_params.max_rx_window = US915_HYBRID_MAX_RX_WINDOW;
    phy_params.recv_delay1 = US915_HYBRID_RECEIVE_DELAY1;
    phy_params.recv_delay2 = US915_HYBRID_RECEIVE_DELAY2;

    phy_params.join_accept_delay1 = US915_HYBRID_JOIN_ACCEPT_DELAY1;
    phy_params.join_accept_delay2 = US915_HYBRID_JOIN_ACCEPT_DELAY2;
    phy_params.max_fcnt_gap = US915_HYBRID_MAX_FCNT_GAP;
    phy_params.ack_timeout = US915_HYBRID_ACKTIMEOUT;
    phy_params.ack_timeout_rnd = US915_HYBRID_ACK_TIMEOUT_RND;
    phy_params.rx_window2_datarate = US915_HYBRID_RX_WND_2_DR;
    phy_params.rx_window2_frequency = US915_HYBRID_RX_WND_2_FREQ;
}

LoRaPHYUS915Hybrid::~LoRaPHYUS915Hybrid()
{
}

void LoRaPHYUS915Hybrid::restore_default_channels()
{
    // Copy channels default mask
    copy_channel_mask(channel_mask, default_channel_mask, US915_HYBRID_CHANNEL_MASK_SIZE);

    for (uint8_t i = 0; i < US915_HYBRID_CHANNEL_MASK_SIZE; i++) {
        // Copy-And the channels mask
        current_channel_mask[i] &= channel_mask[i];
    }
}

bool LoRaPHYUS915Hybrid::get_next_ADR(bool restore_channel_mask, int8_t& dr_out,
                                      int8_t& tx_power_out, uint32_t& adr_ack_cnt)
{
    bool adrAckReq = false;

    uint16_t ack_limit_plus_delay = phy_params.adr_ack_limit + phy_params.adr_ack_delay;

    if (dr_out == phy_params.min_tx_datarate) {
        adr_ack_cnt = 0;
        return adrAckReq;
    }

    if (adr_ack_cnt < phy_params.adr_ack_limit) {
        return adrAckReq;
    }

    // ADR ack counter is larger than ADR-ACK-LIMIT
    adrAckReq = true;
    tx_power_out = phy_params.max_tx_power;


    if (adr_ack_cnt >= ack_limit_plus_delay) {
        if ((adr_ack_cnt % phy_params.adr_ack_delay) == 1) {
            // Decrease the datarate
            dr_out = get_next_lower_tx_datarate(dr_out);

            if (dr_out == phy_params.min_tx_datarate) {
                // We must set adrAckReq to false as soon as we reach the lowest datarate
                adrAckReq = false;
                if (restore_channel_mask) {
                    // Re-enable default channels
                    reenable_500khz_channels(channel_mask[4], channel_mask);
                }
            }
        }
    }

    return adrAckReq;
}

bool LoRaPHYUS915Hybrid::rx_config(rx_config_params_t* config)
{
    int8_t dr = config->datarate;
    uint8_t max_payload = 0;
    int8_t phy_dr = 0;
    uint32_t frequency = config->frequency;

    _radio->lock();

    if (_radio->get_status() != RF_IDLE) {

        _radio->unlock();
        return false;

    }

    _radio->unlock();

    if (config->rx_slot == RX_SLOT_WIN_1) {
        // Apply window 1 frequency
        frequency = US915_HYBRID_FIRST_RX1_CHANNEL + (config->channel % 8) * US915_HYBRID_STEPWIDTH_RX1_CHANNEL;
    }

    // Read the physical datarate from the datarates table
    phy_dr = datarates_US915_HYBRID[dr];

    _radio->lock();

    _radio->set_channel( frequency );

    // Radio configuration
    _radio->set_rx_config(MODEM_LORA, config->bandwidth, phy_dr, 1, 0, 8,
                          config->window_timeout, false, 0, false, 0, 0, true,
                          config->is_rx_continuous);

    _radio->unlock();

    if (config->is_repeater_supported == true) {
        max_payload = max_payloads_with_repeater_US915_HYBRID[dr];
    } else {
        max_payload = max_payloads_US915_HYBRID[dr];
    }

    _radio->lock();
    _radio->set_max_payload_length(MODEM_LORA, max_payload + LORA_MAC_FRMPAYLOAD_OVERHEAD);
    _radio->unlock();

    return true;
}

bool LoRaPHYUS915Hybrid::tx_config(tx_config_params_t* config, int8_t* tx_power,
                                   lorawan_time_t* tx_toa)
{
    int8_t phy_dr = datarates_US915_HYBRID[config->datarate];

    int8_t tx_power_limited = limit_tx_power(config->tx_power,
                                           bands[channels[config->channel].band].max_tx_pwr,
                                           config->datarate);

    uint32_t bandwidth = get_bandwidth (config->datarate);
    int8_t phy_tx_power = 0;

    // Calculate physical TX power
    phy_tx_power = compute_tx_power(tx_power_limited, US915_HYBRID_DEFAULT_MAX_ERP, 0);

    _radio->lock();

    _radio->set_channel( channels[config->channel].frequency );

    _radio->set_tx_config(MODEM_LORA, phy_tx_power, 0, bandwidth, phy_dr, 1, 8,
                          false, true, 0, 0, false, 3000);

    // Setup maximum payload lenght of the radio driver
    _radio->set_max_payload_length(MODEM_LORA, config->pkt_len);

    // Get the time-on-air of the next tx frame
    *tx_toa = _radio->time_on_air(MODEM_LORA, config->pkt_len);

    _radio->unlock();
    *tx_power = tx_power_limited;

    return true;
}

uint8_t LoRaPHYUS915Hybrid::link_ADR_request(adr_req_params_t* params,
                                             int8_t* dr_out, int8_t* tx_power_out,
                                             uint8_t* nb_rep_out,
                                             uint8_t* nb_bytes_parsed)
{
    uint8_t status = 0x07;
    link_adr_params_t adr_settings;
    uint8_t next_idx = 0;
    uint8_t bytes_processed = 0;
    uint16_t temp_channel_mask[US915_HYBRID_CHANNEL_MASK_SIZE] = {0, 0, 0, 0, 0};

    verify_adr_params_t verify_params;

    // Initialize local copy of channels mask
    copy_channel_mask(temp_channel_mask, channel_mask, US915_HYBRID_CHANNEL_MASK_SIZE);

    while (bytes_processed < params->payload_size) {
        next_idx = parse_link_ADR_req(&(params->payload[bytes_processed]),
                                      &adr_settings);

        if (next_idx == 0) {
            break; // break loop, since no more request has been found
        }

        // Update bytes processed
        bytes_processed += next_idx;

        // Revert status, as we only check the last ADR request for the channel mask KO
        status = 0x07;

        if (adr_settings.ch_mask_ctrl == 6) {
            // Enable all 125 kHz channels
            temp_channel_mask[0] = 0xFFFF;
            temp_channel_mask[1] = 0xFFFF;
            temp_channel_mask[2] = 0xFFFF;
            temp_channel_mask[3] = 0xFFFF;
            // Apply chMask to channels 64 to 71
            temp_channel_mask[4] = adr_settings.channel_mask;
        } else if( adr_settings.ch_mask_ctrl == 7 ) {
            // Disable all 125 kHz channels
            temp_channel_mask[0] = 0x0000;
            temp_channel_mask[1] = 0x0000;
            temp_channel_mask[2] = 0x0000;
            temp_channel_mask[3] = 0x0000;
            // Apply chMask to channels 64 to 71
            temp_channel_mask[4] = adr_settings.channel_mask;
        } else if( adr_settings.ch_mask_ctrl == 5 ) {
            // RFU
            status &= 0xFE; // Channel mask KO
        } else {
            temp_channel_mask[adr_settings.ch_mask_ctrl] = adr_settings.channel_mask;
        }
    }

    // FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
    if ((adr_settings.datarate < DR_4) &&
        (num_active_channels( temp_channel_mask, 0, 4 ) < 2)) {
        status &= 0xFE; // Channel mask KO
    }

    if( validate_channel_mask(temp_channel_mask ) == false) {
        status &= 0xFE; // Channel mask KO
    }

    verify_params.status = status;
    verify_params.adr_enabled = params->adr_enabled;
    verify_params.datarate = adr_settings.datarate;
    verify_params.tx_power = adr_settings.tx_power;
    verify_params.nb_rep = adr_settings.nb_rep;
    verify_params.current_datarate = params->current_datarate;
    verify_params.current_tx_power = params->current_tx_power;
    verify_params.current_nb_rep = params->current_nb_rep;
    verify_params.channel_mask = temp_channel_mask;


    // Verify the parameters and update, if necessary
    status = verify_link_ADR_req(&verify_params, &adr_settings.datarate,
                                 &adr_settings.tx_power, &adr_settings.nb_rep);

    // Update channelsMask if everything is correct
    if (status == 0x07) {
        // Copy Mask
        copy_channel_mask(channel_mask, temp_channel_mask, US915_HYBRID_CHANNEL_MASK_SIZE);

        current_channel_mask[0] &= channel_mask[0];
        current_channel_mask[1] &= channel_mask[1];
        current_channel_mask[2] &= channel_mask[2];
        current_channel_mask[3] &= channel_mask[3];
        current_channel_mask[4] = channel_mask[4];
    }

    // Update status variables
    *dr_out = adr_settings.datarate;
    *tx_power_out = adr_settings.tx_power;
    *nb_rep_out = adr_settings.nb_rep;
    *nb_bytes_parsed = bytes_processed;

    return status;
}

uint8_t LoRaPHYUS915Hybrid::accept_rx_param_setup_req(rx_param_setup_req_t* params)
{
    uint8_t status = 0x07;
    uint32_t freq = params->frequency;

    // Verify radio frequency
    if ((_radio->check_rf_frequency(freq) == false)
            || (freq < US915_HYBRID_FIRST_RX1_CHANNEL)
            || (freq > US915_HYBRID_LAST_RX1_CHANNEL)
            || (((freq - ( uint32_t ) US915_HYBRID_FIRST_RX1_CHANNEL) % (uint32_t) US915_HYBRID_STEPWIDTH_RX1_CHANNEL) != 0)) {
        status &= 0xFE; // Channel frequency KO
    }

    // Verify datarate
    if (val_in_range(params->datarate, US915_HYBRID_RX_MIN_DATARATE, US915_HYBRID_RX_MAX_DATARATE) == 0) {
        status &= 0xFD; // Datarate KO
    }

    if ((val_in_range(params->datarate, DR_5, DR_7) == 1)
            || (params->datarate > DR_13)) {
        status &= 0xFD; // Datarate KO
    }

    // Verify datarate offset
    if (val_in_range(params->dr_offset, US915_HYBRID_MIN_RX1_DR_OFFSET, US915_HYBRID_MAX_RX1_DR_OFFSET) == 0) {
        status &= 0xFB; // Rx1DrOffset range KO
    }

    return status;
}

int8_t LoRaPHYUS915Hybrid::get_alternate_DR(uint8_t nb_trials)
{
    int8_t datarate = 0;

    // Re-enable 500 kHz default channels
    reenable_500khz_channels(channel_mask[4], channel_mask);

    if ((nb_trials & 0x01) == 0x01) {
        datarate = DR_4;
    } else {
        datarate = DR_0;
    }

    return datarate;
}

lorawan_status_t LoRaPHYUS915Hybrid::set_next_channel(channel_selection_params_t* params,
                                                      uint8_t* channel, lorawan_time_t* time,
                                                      lorawan_time_t* aggregate_timeOff)
{
    uint8_t nb_enabled_channels = 0;
    uint8_t delay_tx = 0;
    uint8_t enabled_channels[US915_HYBRID_MAX_NB_CHANNELS] = {0};
    lorawan_time_t next_tx_delay = 0;

    // Count 125kHz channels
    if (num_active_channels(current_channel_mask, 0, 4) == 0) {
        // Reactivate default channels
        copy_channel_mask(current_channel_mask, channel_mask, 4);
    }

    // Check other channels
    if (params->current_datarate >= DR_4) {
        if ((current_channel_mask[4] & 0x00FF ) == 0) {
            current_channel_mask[4] = channel_mask[4];
        }
    }

    if (params->aggregate_timeoff <= _lora_time.get_elapsed_time( params->last_aggregate_tx_time)) {
        // Reset Aggregated time off
        *aggregate_timeOff = 0;

        // Update bands Time OFF
        next_tx_delay = update_band_timeoff(params->joined,
                                            params->dc_enabled, bands,
                                            US915_HYBRID_MAX_NB_BANDS);

        // Search how many channels are enabled
        nb_enabled_channels = enabled_channel_count(params->joined,
                                                    params->current_datarate,
                                                    current_channel_mask,
                                                    enabled_channels, &delay_tx);
    } else {
        delay_tx++;
        next_tx_delay = params->aggregate_timeoff - _lora_time.get_elapsed_time(params->last_aggregate_tx_time);
    }

    if (nb_enabled_channels > 0) {

        // We found a valid channel
        *channel = enabled_channels[get_random(0, nb_enabled_channels - 1)];
        // Disable the channel in the mask
        disable_channel(current_channel_mask, *channel, US915_HYBRID_MAX_NB_CHANNELS - 8);

        *time = 0;
        return LORAWAN_STATUS_OK;

    } else {

        if (delay_tx > 0) {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = next_tx_delay;
            return LORAWAN_STATUS_DUTYCYCLE_RESTRICTED;
        }

        // Datarate not supported by any channel
        *time = 0;
        return LORAWAN_STATUS_NO_CHANNEL_FOUND;
    }
}

void LoRaPHYUS915Hybrid::set_tx_cont_mode(cw_mode_params_t* params, uint32_t given_frequency)
{
    (void)given_frequency;

    int8_t tx_power_limited = limit_tx_power(params->tx_power,
                                           bands[channels[params->channel].band].max_tx_pwr,
                                           params->datarate);

    int8_t phy_tx_power = 0;
    uint32_t frequency = channels[params->channel].frequency;

    // Calculate physical TX power
    phy_tx_power = compute_tx_power(tx_power_limited, US915_HYBRID_DEFAULT_MAX_ERP, 0);

    _radio->lock();
    _radio->set_tx_continuous_wave(frequency, phy_tx_power, params->timeout);
    _radio->unlock();
}

uint8_t LoRaPHYUS915Hybrid::apply_DR_offset(int8_t dr, int8_t drOffset)
{
    int8_t datarate = datarate_offsets_US915_HYBRID[dr][drOffset];

    if (datarate < 0) {
        datarate = DR_0;
    }

    return datarate;
}


void LoRaPHYUS915Hybrid::reenable_500khz_channels(uint16_t mask, uint16_t* channelsMask)
{
    uint16_t blockMask = mask;

    for (uint8_t i = 0, j = 0; i < 4; i++, j += 2) {
        channelsMask[i] = 0;
        if ((blockMask & (1 << j)) != 0) {
            channelsMask[i] |= 0x00FF;
        }

        if ((blockMask & (1 << (j + 1))) != 0) {
            channelsMask[i] |= 0xFF00;
        }
    }

    channelsMask[4] = blockMask;
}

int8_t LoRaPHYUS915Hybrid::limit_tx_power(int8_t txPower, int8_t maxBandTxPower,
                                          int8_t datarate)
{
    int8_t txPowerResult = txPower;

    // Limit tx power to the band max
    txPowerResult =  MAX(txPower, maxBandTxPower);

    if (datarate == DR_4) {

        // Limit tx power to max 26dBm
        txPowerResult = MAX(txPower, TX_POWER_2);

    } else {

        if (num_active_channels(channel_mask, 0, 4) < 50) {
            // Limit tx power to max 21dBm
            txPowerResult = MAX(txPower, TX_POWER_5);
        }
    }

    return txPowerResult;
}

bool LoRaPHYUS915Hybrid::validate_channel_mask(uint16_t* channel_masks)
{
    bool mask_state = false;

    uint16_t block1 = 0;
    uint16_t block2 = 0;
    uint8_t index = 0;
    uint16_t temp_channel_masks[US915_HYBRID_CHANNEL_MASK_SIZE];

    // Copy channels mask to not change the input
    for (uint8_t i = 0; i < 4; i++) {
        temp_channel_masks[i] = channel_masks[i];
    }

    for(uint8_t i = 0; i < 4; i++) {
        block1 = temp_channel_masks[i] & 0x00FF;
        block2 = temp_channel_masks[i] & 0xFF00;

        if (count_bits(block1, 16) > 1) {

            temp_channel_masks[i] &= block1;
            temp_channel_masks[4] = 1 << ( i * 2 );
            mask_state = true;
            index = i;
            break;

        } else if( count_bits( block2, 16 ) > 1 ) {

            temp_channel_masks[i] &= block2;
            temp_channel_masks[4] = 1 << ( i * 2 + 1 );
            mask_state = true;
            index = i;
            break;

        }
    }

    // Do change the channel mask, if we have found a valid block.
    if (mask_state == true) {
        // Copy channels mask back again
        for (uint8_t i = 0; i < 4; i++) {
            channel_masks[i] = temp_channel_masks[i];

            if (i != index) {
                channel_masks[i] = 0;
            }
        }

        channel_masks[4] = temp_channel_masks[4];
    }

    return mask_state;
}
