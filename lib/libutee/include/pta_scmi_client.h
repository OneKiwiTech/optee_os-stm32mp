/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2019-2021, Linaro Limited
 */
#ifndef PTA_SCMI_CLIENT_H
#define PTA_SCMI_CLIENT_H

#define PTA_SCMI_UUID { 0xa8cfe406, 0xd4f5, 0x4a2e, \
		{ 0x9f, 0x8d, 0xa2, 0x5d, 0xc7, 0x54, 0xc0, 0x99 } }

#define PTA_SCMI_NAME "PTA-SCMI"

/*
 * PTA_SCMI_CMD_CAPABILITIES - Get channel capabilities
 *
 * [out]    value[0].a: Capabilities bit mask (PTA_SCMI_CAPS_*)
 * [out]    value[0].b: Extended capabilities or 0
 */
#define PTA_SCMI_CMD_CAPABILITIES	0

/*
 * PTA_SCMI_CMD_PROCESS_SMT_CHANNEL - Process SCMI message in SMT buffer
 *
 * [in]     value[0].a: Channel handle
 *
 * Shared memory used for SCMI message/response exhange is expected
 * already identified and bound to channel handle in both SCMI agent
 * and SCMI server (OP-TEE) parts.
 * The memory uses SMT header to carry SCMI meta-data (protocol ID and
 * protocol message ID).
 */
#define PTA_SCMI_CMD_PROCESS_SMT_CHANNEL	1

/*
 * PTA_SCMI_CMD_PROCESS_SMT_CHANNEL_MESSAGE - Process SCMI message in
 *				SMT buffer pointed by memref parameters
 *
 * [in]     value[0].a: Channel handle
 * [in/out] memref[1]: Message/response buffer (SMT and SCMI payload)
 *
 * Shared memory used for SCMI message/response is a SMT buffer
 * referenced by param[1]. It shall be 128 bytes large to fit response
 * payload whatever message playload size.
 * The memory uses SMT header to carry SCMI meta-data (protocol ID and
 * protocol message ID).
 */
#define PTA_SCMI_CMD_PROCESS_SMT_CHANNEL_MESSAGE	2

/*
 * PTA_SCMI_CMD_GET_CHANNEL_HANDLE - Get handle for an SCMI channel
 *
 * Get a handle for the SCMI channel. This handle value is to be passed
 * as argument to some commands as PTA_SCMI_CMD_PROCESS_*.
 *
 * [in]     value[0].a: Channel identifier or 0 if no assigned ID
 * [in]     value[0].b: Requested capabilities mask (PTA_SCMI_CAPS_*)
 * [out]    value[0].a: Returned channel handle
 */
#define PTA_SCMI_CMD_GET_CHANNEL_HANDLE		3

/*
 * PTA_SCMI_CMD_OCALL_THREAD - Allocate a threaded path using OCALL
 *
 * [in]     value[0].a: channel handle
 *
 * Use Ocall support to create a provisioned OP-TEE thread context for
 * the channel. Successful creation of the thread makes this command to
 * return with Ocall command PTA_SCMI_OCALL_CMD_THREAD_READY.
 */
#define PTA_SCMI_CMD_OCALL_THREAD		4

/*
 * Capabilities
 */

/* Channel supports shared memory using the SMT header protocol */
#define PTA_SCMI_CAPS_SMT_HEADER			BIT32(0)

/*
 * Channel can use command PTA_SCMI_CMD_OCALL_THREAD to provision a
 * TEE thread for SCMI message passing.
 */
#define PTA_SCMI_CAPS_OCALL_THREAD			BIT32(1)

#define PTA_SCMI_CAPS_VALID_MASK	(PTA_SCMI_CAPS_SMT_HEADER | \
					 PTA_SCMI_CAPS_OCALL_THREAD)

/*
 * enum optee_scmi_ocall_cmd
 * enum optee_scmi_ocall_reply
 *
 * These enumerates define the IDs used by REE/TEE to communicate in the
 * established REE/TEE Ocall thread context.
 *
 * At channel setup, we start from the REE: caller requests an Ocall context.
 *
 * 0. REE opens a session toward PTA SCMI. REE invokes PTA command
 *    PTA_SCMI_CMD_GET_CHANNEL to get a channel handler.
 *
 * 1. REE invokes command PTA_SCMI_CAPS_OCALL_THREAD with an Ocall context.
 *    This is the initial invocation of the Ocall thread context. Any further
 *    error in the thread communication will make Core to return to REE from
 *    this command invocation with an TEE_Result error code.
 *
 * 2. Upon support of Ocall the PTA creates an Ocall context and returns to
 *    REE with Ocall command PTA_SCMI_OCALL_CMD_THREAD_READY.
 *
 * 3. REE returns to the PTA, from the Ocall, with output param[0].value.a
 *    set to PTA_SCMI_OCALL_PROCESS_SMT_CHANNEL to post an SCMI message.
 *    In such case, OP-TEE processes the message and returns to REE with
 *    Ocall command PTA_SCMI_OCALL_CMD_THREAD_READY. The SCMI response is in
 *    the shared memory buffer.
 *
 * 4. Alternatively REE can return from the Ocall with output param[0].value.a
 *    set to PTA_SCMI_OCALL_CLOSE_THREAD. This requests OP-TEE to terminate the
 *    Ocall, release resources and return from initial command invocation at 1.
 *    as if REE closes the SCMI communication.
 *
 * At anytime, if an error is reported by Ocall commands or replies, SCMI PTA
 * release the Ocall thread context and return from initial invocation at 1.
 * PTA_SCMI_OCALL_ERROR is used in Ocall return to force an error report.
 *
 * REE channel initialization completes when returning from step 2.
 * REE agent posts an SCMI message through step 3.
 * At channel release, REE driver executes step 4.
 */

enum optee_scmi_ocall_cmd {
	PTA_SCMI_OCALL_CMD_THREAD_READY = 0,
};

enum optee_scmi_ocall_reply {
	PTA_SCMI_OCALL_ERROR = 0,
	PTA_SCMI_OCALL_CLOSE_THREAD = 1,
	PTA_SCMI_OCALL_PROCESS_SMT_CHANNEL = 2,
};
#endif /* SCMI_PTA_SCMI_CLIENT_H */
