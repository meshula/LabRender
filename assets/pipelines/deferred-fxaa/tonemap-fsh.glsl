half4 ApplyCameraEffects(PrecisionVec4 final_output, AAPLFrameUniforms frameUniforms) {
    final_output += PrecisionVec4(PrecisionVec3(frameUniforms.scene.flash), 1);
    final_output *= PrecisionVec4(PrecisionVec3(frameUniforms.scene.exposure), 1);
    
    // clamp max output to compression max
    final_output = min(final_output, PrecisionVec4(frameUniforms.scene.exposureCompression.y));
    
    // preserve the linear portion
    PrecisionVec4 linear_portion = min(final_output, PrecisionVec4(frameUniforms.scene.exposureCompression.x));
    
    // compress the remainder
    PrecisionVec4 nonlinear_portion = final_output - linear_portion;
    float normalizeVal = max(frameUniforms.scene.exposureCompression.y - frameUniforms.scene.exposureCompression.x, 0.001); // prevent divide by 0
    nonlinear_portion /= normalizeVal; // normalize 0 - 1
    nonlinear_portion = log(nonlinear_portion * 3 + PrecisionVec4(1))/ PrecisionVec4(0.677); // ad hoc compression curve
    nonlinear_portion *= (1.0 - frameUniforms.scene.exposureCompression.x); // normalize to screen values 0 - 1, above the knee
    
    final_output = linear_portion; // + nonlinear_portion;
    return half4(clamp(float4(final_output), 0.0, 1.0));
}
