/*----------------------------------------------------------------------------*/
/* Linker Settings                                                            */
--retain="*(.intvecs)"

--stack_size=0x1000
/*----------------------------------------------------------------------------*/
/* Section Configuration                                                      */
SECTIONS
{
    systemHeap : {} >> L2SRAM_UMAP0 | L2SRAM_UMAP1
    .l2data : {} >> L2SRAM_UMAP0 | L2SRAM_UMAP1

    /* L3SRAM has code that is overlaid with data, so data must be
       marked uninitialized. Application can initialize this section
       using _L3data_* symbols defined below. Code should be written carefully as
       these are linker symbols (see for example http://e2e.ti.com/support/development_tools/compiler/f/343/t/92002 ):
        
        extern far uint8_t _L3data_start; // the type here does not matter
        extern far uint8_t _L3data_size;  // the type here does not matter

        memset((void *)_symval(&_L3data_start), 0, (uint32_t) _symval(&_L3data_size));
    */ 
    .l3data: type=NOINIT, start(_L3data_start), size(_L3data_size), load=L3SRAM PAGE 1

    /* Bootloader cannot load L1DSRAM, make sure to mark as NOINIT */
    .l1data : type=NOINIT, load=L1DSRAM

    /* memory section defined in HSRAM */
    .hsramdata : type=NOINIT, load=HSRAM

    /* Currently bootloader does not allow loading in L1PSRAM because of supporting
       low power. Below fast code is loaded in L3SRAM but run from L1PSRAM. The copy-in
       is called during initialization phase and copy-out is not used but can be added when
       supporting low-power mode (where L1 contents are not retained).
	   
	   The functions listed in this section are paricularly picked because they contains haeavy calculation loops.
     */    
    .fastCode: 
    {
		-ldsplib.lib (.text:DSPF_sp_fftSPxSP)
		RADARDEMO_highAccuRangeProc_priv.oe674(.text:RADARDEMO_highAccuRangeProc_rangeEst)
		RADARDEMO_highAccuRangeProc.oe674(.text:RADARDEMO_highAccuRangeProc_create)
		RADARDEMO_highAccuRangeProc_priv.oe674(.text:RADARDEMO_highAccuRangeProc_accumulateInput)
		RADARDEMO_highAccuRangeProc.oe674(.text:RADARDEMO_highAccuRangeProc_run)
		dss_main.oe674 (.text:MmwDemo_dssDataPathProcessEvents)
		dss_data_path.oe674 (.text:MmwDemo_interFrameProcessing)
		dss_data_path.oe674 (.text:MmwDemo_processChirp)
		
    } load=L3SRAM PAGE 0, run=L1PSRAM PAGE 0, table(_MmwDemo_fastCode_L1PSRAM_copy_table, compression=off)
    
    /* This is auto generated by linker related to copy table above */
    .ovly > L2SRAM_UMAP0 | L2SRAM_UMAP1
    
    /* Overlay one-time/init-time (and non-critical in cycles) with L3 data,
       will be erased during data path processing. Note do not put any
       code that is required related to start/stop/reconfig processing 
	   
	   The RADARDEMO function in this section are the ones in the code base as algorithm options but never uses for TM demo. 
	   For other signal chains that may use the functions listed below, they have to be removed from the overlay section, otherwise
	   the code will be overwrite at runtime by radar cude or other data buffer and will casue crash.
	   */
    .overlay:
    {
    //    -llibsoc_xwr16xx.ae674 (.text:SOC_init)
        dss_main.oe674 (.text:MmwDemo_dssInitTask)
        dss_main.oe674 (.text:main)
        dss_data_path.oe674 (.text:MmwDemo_dataPathInitEdma)
		
    } > L3SRAM PAGE 0
   
}
/*----------------------------------------------------------------------------*/

