
// unsigned char strlen( unsigned char* str ) {
// 	unsigned char len = 0;
// 	while ( str[len] != 0 ) ++len;
// 	return len;
// }

unsigned char main(void) {
	// char str[] = "Hello World!";
	// unsigned char len = strlen(str);
	__asm__("lda 'A'");
	__asm__("sta $D010");
	__asm__("lda $01");
	__asm__("sta $D011");
	

	return 0;
}