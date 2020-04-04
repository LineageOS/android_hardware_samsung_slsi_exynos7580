r remaining input */
  memcpy(&context->buffer[bufindex], &input[i], inputLen-i);
}

/* MD4 padding. */
static void MD4Pad(MD4_CTX *context)
{
  unsigned char bits[8];
  unsigned int bufindex, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
   */
  bufindex = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (bufindex < 56) ? (56 - bufindex) : (120 - bufindex);
  MD4Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD4Update (context, bits, 8);
}

/* MD4 finalization. Ends an MD4 message-digest operation, writing the
     the message digest and zeroizing the context.
 */
static void MD4Final (unsigned char digest[16], MD4_CTX *context)
{
