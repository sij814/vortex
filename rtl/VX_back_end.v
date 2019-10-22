module VX_back_end (
	input wire clk, 
	input wire reset, 
	input wire schedule_delay,

	output wire               out_mem_delay,
	output wire               gpr_stage_delay,
	VX_jal_response_inter     VX_jal_rsp,
	VX_branch_response_inter  VX_branch_rsp,


	VX_frE_to_bckE_req_inter  VX_bckE_req,
	VX_wb_inter               VX_writeback_inter,

	VX_warp_ctl_inter         VX_warp_ctl,

	VX_dcache_response_inter  VX_dcache_rsp,
	VX_dcache_request_inter   VX_dcache_req

);


VX_wb_inter             VX_writeback_temp();
assign VX_writeback_inter.wb           = VX_writeback_temp.wb;
assign VX_writeback_inter.rd           = VX_writeback_temp.rd;
assign VX_writeback_inter.write_data   = VX_writeback_temp.write_data;
assign VX_writeback_inter.wb_valid     = VX_writeback_temp.wb_valid;
assign VX_writeback_inter.wb_warp_num  = VX_writeback_temp.wb_warp_num;

// assign VX_writeback_inter(VX_writeback_temp);


VX_mw_wb_inter           VX_mw_wb();


VX_mem_req_inter  VX_exe_mem_req();
VX_mem_req_inter  VX_mem_req();



// LSU input + output
VX_lsu_req_inter         VX_lsu_req();
VX_inst_mem_wb_inter     VX_mem_wb();

// Exec unit input + output
VX_exec_unit_req_inter   VX_exec_unit_req();
VX_inst_exec_wb_inter    VX_inst_exec_wb();


// GPU unit input
VX_gpu_inst_req_inter    VX_gpu_inst_req();

// CSR unit inputs
VX_csr_req_inter         VX_csr_req();
VX_csr_wb_inter          VX_csr_wb();

VX_gpr_stage VX_gpr_stage(
	.clk               (clk),
	.reset             (reset),
	.schedule_delay    (schedule_delay),
	.VX_writeback_inter(VX_writeback_temp),
	.VX_bckE_req       (VX_bckE_req),
	// New
	.VX_exec_unit_req(VX_exec_unit_req),
	.VX_lsu_req      (VX_lsu_req),
	.VX_gpu_inst_req (VX_gpu_inst_req),
	.VX_csr_req      (VX_csr_req),
	// End new
	.memory_delay      (out_mem_delay),
	.gpr_stage_delay   (gpr_stage_delay)
	);


VX_lsu load_store_unit(
	// .clk          (clk),
	.VX_lsu_req   (VX_lsu_req),
	.VX_mem_wb    (VX_mem_wb),
	.VX_dcache_rsp(VX_dcache_rsp),
	.VX_dcache_req(VX_dcache_req),
	.out_delay    (out_mem_delay)
	);


VX_execute_unit VX_execUnit(
	// .clk             (clk),
	.VX_exec_unit_req(VX_exec_unit_req),
	.VX_inst_exec_wb (VX_inst_exec_wb),
	.VX_jal_rsp      (VX_jal_rsp),
	.VX_branch_rsp   (VX_branch_rsp)
	);


VX_gpgpu_inst VX_gpgpu_inst(
	.VX_gpu_inst_req(VX_gpu_inst_req),
	.VX_warp_ctl    (VX_warp_ctl)
	);

VX_csr_wrapper VX_csr_wrapper(
	.VX_csr_req(VX_csr_req),
	.VX_csr_wb (VX_csr_wb)
	);

VX_writeback VX_wb(
	.VX_mem_wb         (VX_mem_wb),
	.VX_inst_exec_wb   (VX_inst_exec_wb),
	.VX_csr_wb         (VX_csr_wb),

	.VX_writeback_inter(VX_writeback_temp)
	);

endmodule