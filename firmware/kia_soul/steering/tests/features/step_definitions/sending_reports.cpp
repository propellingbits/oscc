WHEN("^a steering report is published$")
{
    g_steering_control_state.enabled = true;
    g_steering_control_state.operator_override = true;
    g_steering_control_state.dtcs = 0x55;

    publish_steering_report();
}


WHEN("^a fault report is published$")
{
    publish_fault_report();
}


THEN("^a steering report should be put on the control CAN bus$")
{
    assert_that(g_mock_mcp_can_send_msg_buf_id, is_equal_to(OSCC_STEERING_REPORT_CAN_ID));
    assert_that(g_mock_mcp_can_send_msg_buf_ext, is_equal_to(CAN_STANDARD));
    assert_that(g_mock_mcp_can_send_msg_buf_len, is_equal_to(OSCC_STEERING_REPORT_CAN_DLC));
}


THEN("^a fault report should be put on the control CAN bus$")
{
    assert_that(g_mock_mcp_can_send_msg_buf_id, is_equal_to(OSCC_FAULT_REPORT_CAN_ID));
    assert_that(g_mock_mcp_can_send_msg_buf_ext, is_equal_to(CAN_STANDARD));
    assert_that(g_mock_mcp_can_send_msg_buf_len, is_equal_to(OSCC_FAULT_REPORT_CAN_DLC));
}


THEN("^the steering report's enabled field should be set$")
{
    oscc_steering_report_s * steering_report =
        (oscc_steering_report_s *) g_mock_mcp_can_send_msg_buf_buf;

    assert_that(
        steering_report->enabled,
        is_equal_to(g_steering_control_state.enabled));
}


THEN("^the steering report's override field should be set$")
{
    oscc_steering_report_s * steering_report =
        (oscc_steering_report_s *) g_mock_mcp_can_send_msg_buf_buf;

    assert_that(
        steering_report->operator_override,
        is_equal_to(g_steering_control_state.operator_override));
}


THEN("^the steering report's DTCs field should be set$")
{
    oscc_steering_report_s * steering_report =
        (oscc_steering_report_s *) g_mock_mcp_can_send_msg_buf_buf;

    assert_that(
        steering_report->dtcs,
        is_equal_to(g_steering_control_state.dtcs));
}


THEN("^the fault report's origin ID field should be set$")
{
    oscc_fault_report_s * fault_report =
        (oscc_fault_report_s *) g_mock_mcp_can_send_msg_buf_buf;

    assert_that(
        fault_report->fault_origin_id,
        is_equal_to(FAULT_ORIGIN_STEERING));
}
