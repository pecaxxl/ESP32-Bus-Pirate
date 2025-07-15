#include "I2sService.h"
#include "math.h"

void I2sService::configureOutput(uint8_t bclk, uint8_t lrck, uint8_t dout, uint32_t sampleRate, uint8_t bits) {
    if (initialized) i2s_driver_uninstall(port);

    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)(bits),
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pins = {
        .bck_io_num = bclk,
        .ws_io_num = lrck,
        .data_out_num = dout,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(port, &config, 0, nullptr);
    i2s_set_pin(port, &pins);

    initialized = true;
}

void I2sService::configureInput(uint8_t bclk, uint8_t lrck, uint8_t din, uint32_t sampleRate, uint8_t bits) {
    if (initialized) i2s_driver_uninstall(port);

    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)(bits),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pins = {
        .bck_io_num = bclk,
        .ws_io_num = lrck,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = din
    };

    i2s_driver_install(port, &config, 0, nullptr);
    i2s_set_pin(port, &pins);

    initialized = true;
}

void I2sService::playTone(uint16_t freq, uint16_t durationMs) {
    if (!initialized) return;

    const int sampleRate = 44100;
    int samples = (sampleRate * durationMs) / 1000;
    int16_t buffer[2];

    for (int i = 0; i < samples; ++i) {
        float t = (float)i / sampleRate;
        int16_t sample = sinf(2.0f * PI * freq * t) * 32767;
        buffer[0] = sample;
        buffer[1] = sample;
        size_t written;
        i2s_write(port, buffer, sizeof(buffer), &written, portMAX_DELAY);
    }
}

void I2sService::playToneInterruptible(uint16_t freq, uint32_t durationMs, std::function<bool()> shouldStop) {
    const int sampleRate = 44100;
    const int16_t amplitude = 32767;
    const int chunkDurationMs = 20;
    const int samplesPerChunk = (sampleRate * chunkDurationMs) / 1000;
    int16_t buffer[2 * samplesPerChunk];

    uint32_t elapsed = 0;
    while (elapsed < durationMs) {
        for (int i = 0; i < samplesPerChunk; ++i) {
            float t = (float)(i + (elapsed * sampleRate / 1000)) / sampleRate;
            int16_t sample = sinf(2.0f * PI * freq * t) * amplitude;
            buffer[2 * i] = sample;
            buffer[2 * i + 1] = sample;
        }

        size_t written;
        i2s_write(port, buffer, sizeof(buffer), &written, portMAX_DELAY);
        elapsed += chunkDurationMs;

        if (shouldStop()) {
            break;
        }
    }
}

void I2sService::playPcm(const int16_t* data, size_t numBytes) {
    if (!initialized || data == nullptr || numBytes == 0) return;

    size_t bytesWritten = 0;
    i2s_write(port, data, numBytes, &bytesWritten, portMAX_DELAY);
}

size_t I2sService::recordSamples(int16_t* outBuffer, size_t sampleCount) {
    if (!initialized) return 0;

    size_t totalRead = 0;
    size_t bytesToRead = sampleCount * sizeof(int16_t);

    uint8_t* buffer = reinterpret_cast<uint8_t*>(outBuffer);
    while (totalRead < bytesToRead) {
        size_t readBytes = 0;
        i2s_read(port, buffer + totalRead, bytesToRead - totalRead, &readBytes, portMAX_DELAY);
        totalRead += readBytes;
    }

    return totalRead / sizeof(int16_t);
}

void I2sService::stop() {
    if (initialized) {
        i2s_driver_uninstall(port);
        initialized = false;
    }
}

bool I2sService::isInitialized() const {
    return initialized;
}
