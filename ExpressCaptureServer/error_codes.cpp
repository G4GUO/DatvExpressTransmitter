#include "stdafx.h"
#include "error_codes.h"

int m_error;

void report_error(int code) {
	m_error = code;
}

int get_error(void) {
	return m_error;
}