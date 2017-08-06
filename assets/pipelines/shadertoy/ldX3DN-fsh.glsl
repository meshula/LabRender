void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	// Fragment coords relative to the center of viewport, in a 1 by 1 coords sytem.
	vec2 uv = -1.0 + 2.0* fragCoord.xy / iResolution.xy;

	// But I want circles, not ovales, so I adjust y with x resolution.
	vec2 homoCoords = vec2( uv.x, 2.0* fragCoord.y / iResolution.x );

	// Sin of distance from a moving origin to current fragment will give us..... 
	vec2 movingOrigin1 = vec2(sin(iGlobalTime*.7),+sin(iGlobalTime*1.7));
	
	// ...numerous... 
	float frequencyBoost = 50.0; 
	
	// ... awesome concentric circles.
	float wavePoint1 = sin(distance(movingOrigin1, homoCoords)*frequencyBoost);
	
	// I want sharp circles, not blurry ones.
	float blackOrWhite1 = sign(wavePoint1);
	
	// That was cool ! Let's do it again ! (No, I dont want to write a function today, I'm tired).
	vec2 movingOrigin2 = vec2(-cos(iGlobalTime*2.0),-sin(iGlobalTime*3.0));
	float wavePoint2 = sin(distance(movingOrigin2, homoCoords)*frequencyBoost);
	float blackOrWhite2 = sign(wavePoint2);
	
	// I love pink.
	vec3 pink = vec3(1.0, .5, .9 );
	vec3 darkPink = vec3(0.5, 0.1, 0.3);
	
	// XOR virtual machine.
	float composite = blackOrWhite1 * blackOrWhite2;
	
	// Pinkization
	fragColor = vec4(max( pink * composite, darkPink), 1.0);
}
