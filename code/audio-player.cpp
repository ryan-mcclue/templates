// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): A NORETURN function might emit warning if calls other non-NORETURN functions
NORETURN INTERNAL void 
gui_fatal_error(const char *attempt, const char *reason, const char *resolution) 
{
  char message_box_buf[1024] = ZERO_STRUCT;
  snprintf(message_box_buf, sizeof(message_box_buf), "%s\n%s\n%s", attempt, reason, resolution);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FATAL ERROR", message_box_buf, NULL); // todo: attach to window

  FATAL_ERROR(attempt, reason, resolution);
}

// TODO(Ryan): Introduced GUI_ASSERT, i.e. SDL_assert();

// IMPORTANT(Ryan): Make incremental changes so as to maintain momentum working 
INTERNAL void 
audio_player(void)
{
  SDL_AudioSpec wave_spec = ZERO_STRUCT;
  wave_spec.freq = 4080;
  wave_spec.format = AUDIO_F32; // most cards don't support, but SDL will convert for us. better for effects
  wave_spec.channels = 2;
  wave_spec.samples = 4096;
  wave_spec.callback = NULL;

  SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &wave_spec, NULL, 0);
  if (audio_device == 0)
  {
    gui_fatal_error("Failed to load audio device", SDL_GetError(), "");
  }

  u8 *wave_buf = NULL;
  u32 wave_len = 0;
  u32 wave_pos = 0;
  // now this will probably not be in f32, so will require conversion
  // so as to not sound scratchy
  if (SDL_LoadWAV("music.wav", &wave_spec, &wave_buf, &wave_len) == NULL)
  {
    gui_fatal_error("Failed to load music.wav", SDL_GetError(), "");
  }

  SDL_AudioStream *audio_stream = \
    SDL_NewAudioStream(wave_spec.format, wave_spec.channels, wave_spec.freq, AUDIO_F32, 2, 48000);
  if (audio_stream == NULL)
  {

  }

  SDL_AudioStreamPut(audio_stream, wave_buf, wave_len);
  // convert all at once, i.e. don't wait for more to be put in queue
  SDL_AudioStreamFlush(audio_stream);

  // returns bytes; confusing nomenclature with frames/samples etc.
  if (SDL_GetQueuedAudioSize(audio_device) < KB(8))
  {
    u32 bytes_remaining = SDL_AudioStreamAvailable(audio_stream);
    if (bytes_remaining > 0)
    {
      u32 bytes_to_write = MIN(bytes_remaining, KB(32));
      u8 converted_audio_bytes[KB(32)] = ZERO_STRUCT;
      SDL_AudioStreamGet(audio_stream, converted_audio_bytes, bytes_to_write);
      SDL_QueueAudio(audio_device, converted_audio_bytes, bytes_to_write);
    }
  }



  SDL_QueueAudio(audio_device, wave_buf, wave_len);
  // as queue audio makes copy
  SDL_FreeWAV(wave_buf);
  
  SDL_FreeAudioStream(audio_stream);

  // IMPORTANT(Ryan): Only fatal_error() on init, never whilst program is running
  
  // for rewind:
  // SDL_ClearQueuedAudio(audio_device); 
  // SDL_AudioStreamClear()
  // SDL_AudioStreamPut(whole_wav_buffer)
  // SDL_AudioStreamFlush()
  
  // SDL_PointInRect();

  SDL_PauseAudioDevice(audio_device, 0); // pauses progression of audio queue

  SDL_Delay(5000);

  SDL_CloseAudioDevice(audio_device);
}
