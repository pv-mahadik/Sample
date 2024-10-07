function decompressed_data = call_mexed_decoder(compressed_data,len_data,k_range, ...
                                           desiredCompressedSize_bits, block_size, ...
                                           bits_to_drop_bw, sample_bw)
%% Calls the 'C' based decoder.
%  Note : Provide all the inputs specified in the function. 
%  The Inputs are :-
%  1) compressed_data : an array of unsigned 32 bit numbers generated by the encoder
%  2) len_data : the length of the decompressed output
%  3) k_range : the list of golomb parameters to search for. 
%  4) block_size : the number of samples per block. 
%  5) desired_compression_ratio and data_bitwidth are used to compute the
%  average bit width of the compressed data. The average bitwidth of the
%  compressed data is given by data_bitwidth*desired_compression_ratio.

    

  n_samples = len_data;
  X.n_samps_input = cast(n_samples,'uint32');
  X.desired_op_size_bits = cast(desiredCompressedSize_bits,'uint32');
  X.num_samps_per_blk = cast(block_size,'uint32');
  X.k_array = cast(k_range,'uint32');
  len_k_range = length(k_range);
    
  X.k_array_bw = cast(log2(len_k_range),'uint32');
  X.bits_to_drop_bw = cast(bits_to_drop_bw,'uint32');  % TODO make it programmable. 
  X.encode_or_decode = cast(0,'uint32'); % 1 -> encode, 0 -> decode.
  X.data = cast(compressed_data,'uint32');

  % Since there is no need for dithering during decompression but the 
  % interface is similar for both decoding and encoding, we populate
  % dummy parameters into the dither variables. 
  X.dither_bw = cast(0,'uint32');
  X.sample_bw = cast(sample_bw,'uint32'); % Unused compression
  X.dither_signal = cast(zeros([n_samples,1]), 'int32');

  decompressed_data = ExpGolombEncDec(X);
  
  decompressed_data = cast(decompressed_data,'double');
  
  decompressed_data = decompressed_data + (decompressed_data >= 2^31)*(-2^32);
end