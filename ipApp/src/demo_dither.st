program demo_dither

ss dither {
  state this {
    when (delay(5)) {
      printf("this");
    } state that
  }

  state that {
    when (delay(5)) {
      printf("that");
    } state this
  }
}
