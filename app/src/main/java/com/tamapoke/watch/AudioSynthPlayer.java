package com.tamapoke.watch;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

final class AudioSynthPlayer {
    private static final int SAMPLE_RATE = 16000;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private AudioTrack track;

    void play(short[] samples) {
        if (executor.isShutdown() || samples == null || samples.length == 0) return;
        executor.execute(() -> {
            if (track == null) {
                int minBuf = AudioTrack.getMinBufferSize(
                        SAMPLE_RATE, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);
                int bufBytes = Math.max(Math.max(minBuf, 2048), SAMPLE_RATE);
                AudioTrack candidate = new AudioTrack.Builder()
                        .setAudioAttributes(new AudioAttributes.Builder()
                                .setUsage(AudioAttributes.USAGE_GAME)
                                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                                .build())
                        .setAudioFormat(new AudioFormat.Builder()
                                .setSampleRate(SAMPLE_RATE)
                                .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                                .build())
                        .setBufferSizeInBytes(bufBytes)
                        .setTransferMode(AudioTrack.MODE_STREAM)
                        .build();
                if (candidate.getState() != AudioTrack.STATE_INITIALIZED) {
                    candidate.release();
                    return;
                }
                track = candidate;
                track.play();
            }
            track.write(samples, 0, samples.length);
        });
    }

    void release() {
        executor.shutdownNow();
        if (track != null) {
            try {
                track.release();
            } catch (IllegalStateException ignored) {
            }
            track = null;
        }
    }
}
