#include "stdafx.h"
#include "ReconvTextStore.h"


//=============================================================================


//! �I�u�W�F�N�g�̐���
HRESULT ReconvTextStore::CreateInstance(ReconvTextStore*& o_text_store_cp)
{
	HRESULT		retval = E_FAIL;

	try
	{
		o_text_store_cp = new ReconvTextStore();
		if( o_text_store_cp != NULL )
			retval = S_OK;
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "exception occured.\n");
	}
	
	return retval;
}


//=============================================================================


//! �f�X�g���N�^
ReconvTextStore::~ReconvTextStore()
{
	// �V���N���o�^����Ă���ꍇ�͉�����Ă����B
	RELEASE(m_sink_cp);
}


//=============================================================================


//! �h�L�������g�̃��b�N���s���B���������ꍇ true, ���s�����ꍇ false ��Ԃ��B
bool ReconvTextStore::LockDocument(DWORD i_lock_flags)
{
	bool		retval			= false;
	LONG		lock_count	= InterlockedIncrement(&m_lock_count);
	
	if( lock_count == 1 )
	{
		m_lock_flags	= i_lock_flags;
		retval				= true;
	}
	else
	{
		InterlockedDecrement(&m_lock_count);
	}
	
	return retval;
}


//=============================================================================


//! �h�L�������g�̃��b�N����������B���b�N���ĂȂ��󋵂ŌĂ΂Ȃ����ƁB
bool ReconvTextStore::UnlockDocument()
{
	bool		retval			= false;
	
	if( m_lock_flags )
	{
		m_lock_flags = 0;
		retval = true;
		InterlockedDecrement(&m_lock_count);
	}
	
	return retval;
}


//=============================================================================


//! �h�L�������g�����b�N�����H
bool ReconvTextStore::IsLocked(DWORD i_lock_flags)
{
	return (m_lock_flags & i_lock_flags) == i_lock_flags;
}


//=============================================================================


//! �ϊ���⃊�X�g���擾��������������Z�b�g����B
bool ReconvTextStore::SetString(const wchar_t* i_string)
{
	bool		retval			= false;
	
	do
	{
		if( IsBadReadPtr(i_string, sizeof(wchar_t)) )
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** i_string is bad ptr.\n");
			break;
		}

		if( LockDocument(TS_LF_READWRITE) == false )
		{
			_RPTF0(_CRT_WARN, "*** Error *** unable to lock.\n");
			break;
		}

		TS_TEXTCHANGE	text_change = { 0 };
		text_change.acpStart  = 0;
		text_change.acpOldEnd = lstrlen(m_text);
		text_change.acpNewEnd = lstrlen(i_string);
		
		lstrcpyn(m_text, i_string, numberof(m_text) - 1);
		UnlockDocument();
			
		if( m_sink_cp && (m_sink_mask & TS_AS_TEXT_CHANGE) )
			m_sink_cp->OnTextChange(0, &text_change);

		retval = true;
	}
	while( 0 );
	
	return retval;
}


//=============================================================================
// IUnknown ����p�����郁�\�b�h
//=============================================================================


//! �Q�ƃJ�E���g�̃C���N�������g
STDMETHODIMP_(DWORD) ReconvTextStore::AddRef()
{
	return ++m_ref_count;
}


//=============================================================================


//! �Q�ƃJ�E���g�̃f�N�������g����щ��
STDMETHODIMP_(DWORD) ReconvTextStore::Release()
{
	DWORD	retval = --m_ref_count;
	
	if( retval == 0 )
		delete this;

	return retval;
}


//=============================================================================


//! �C���^�[�t�F�C�X�̖₢���킹
STDMETHODIMP ReconvTextStore::QueryInterface(
	REFIID		i_riid,
	LPVOID*		o_interface_cp_p
)
{
	HRESULT		retval = E_FAIL;

	do
	{
		if( IsBadWritePtr(o_interface_cp_p, sizeof(LPVOID)) )
		{
			retval = E_INVALIDARG;
			break;
		}

		if( IsEqualIID(i_riid, IID_IUnknown) || IsEqualIID(i_riid, IID_ITextStoreACP) )
		{
			retval = S_OK;
			*o_interface_cp_p = (ITextStoreACP*)this;
			// ���������ꍇ�͎Q�ƃJ�E���g�𑝂₷
			AddRef();
		}
		else
		{
			*o_interface_cp_p = NULL;
			retval = E_NOINTERFACE;
		}
	}
	while( 0 );
	
	return retval;
}


//=============================================================================


//! ��̃I�u�W�F�N�g���������̂��ǂ������ׂ�B
inline bool IsEqualObject(IUnknown* i_lhs, IUnknown* i_rhs)
{
	bool		retval = false;
	
	if( IsBadReadPtr(i_lhs, sizeof(IUnknown*)) == FALSE
	&&	IsBadReadPtr(i_rhs, sizeof(IUnknown*)) == FALSE )
	{
		IUnknown*		lhs_unknown_cp = NULL;
		IUnknown*		rhs_unknown_cp = NULL;

		i_lhs->QueryInterface(IID_IUnknown, (void**)&lhs_unknown_cp);
		i_rhs->QueryInterface(IID_IUnknown, (void**)&rhs_unknown_cp);
		
		retval = (lhs_unknown_cp && lhs_unknown_cp == rhs_unknown_cp);
		
		RELEASE(lhs_unknown_cp);
		RELEASE(rhs_unknown_cp);
	}
	
	return retval;
}


//=============================================================================
// ITextStoreACP ����p�����郁�\�b�h
//=============================================================================


//! �V���N�̓o�^���s���B
STDMETHODIMP ReconvTextStore::AdviseSink(
	REFIID			i_riid,
	IUnknown*		io_unknown_cp,
	DWORD				i_mask
)
{
	HRESULT		retval		= E_FAIL;
	
	do
	{
		// io_unknown_cp �ɐ������|�C���^���n���ꂽ���`�F�b�N����B
		// �܂��A�o�^�ł���V���N�� ITextStoreACPSink �����B
		if( IsBadReadPtr(io_unknown_cp, sizeof(IUnknown*))
		||	IsEqualIID(i_riid, IID_ITextStoreACPSink) == false )
		{
			_RPTF0(_CRT_WARN, "*** Error *** bad ptr or object.\n");
			retval = E_INVALIDARG;
			break;
		}
		
		// ���ɓo�^����Ă���V���N�Ɠ����ꍇ�̓}�X�N�̍X�V���s���B
		if( IsEqualObject(m_sink_cp, io_unknown_cp) )
		{
			m_sink_mask = i_mask;
			retval = S_OK;
		}
		// ���ɃV���N���o�^����Ă���ꍇ�͎��s�B
		else if( m_sink_cp )
		{
			_RPTF0(_CRT_WARN, "*** Error *** sink already exists.\n");
			retval = CONNECT_E_ADVISELIMIT;
		}
		// �V�����o�^
		else
		{
			// ITextStoreACPSink �C���^�[�t�F�C�X���擾����B
			retval = io_unknown_cp->QueryInterface(
								 IID_ITextStoreACPSink,
								 (void**)&m_sink_cp
							 );
			// ���������ꍇ�̓}�X�N��ۑ�����B
			if( SUCCEEDED(retval) )
				m_sink_mask = i_mask;
		}
	}
	while( 0 );
	
	return retval;
}


//=============================================================================


//! �V���N�̓o�^����������B
STDMETHODIMP ReconvTextStore::UnadviseSink(
	IUnknown*		io_unknown_cp
)
{
	HRESULT			retval			= E_FAIL;
	IUnknown*		unk_id_cp		= NULL;
	
	do
	{
		// �L���ȃ|�C���^���n���ꂽ�����ׂ�B
		if( IsBadReadPtr(io_unknown_cp, sizeof(void*)) )
		{
			_RPTF0(_CRT_WARN, "*** Error *** io_unknown_cp is bad ptr.\n");
			retval = E_INVALIDARG;
			break;
		}
	
		// �o�^����Ă��Ȃ��V���N�̏ꍇ�̓G���[�B
		if( IsEqualObject(io_unknown_cp, m_sink_cp) == false )
		{
			_RPTF0(_CRT_WARN, "*** Error *** no connection.\n");
			retval = CONNECT_E_NOCONNECTION;
			break;
		}

		// ���
		RELEASE(m_sink_cp);
		m_sink_mask = 0;
		retval = S_OK;
	}
	while( 0 );
	
	return retval;
}


//=============================================================================


//! �h�L�������g�����b�N���ēo�^�ς݃V���N�̃��\�b�h���R�[������B
STDMETHODIMP ReconvTextStore::RequestLock(
	DWORD				i_lock_flags,
	HRESULT*		o_session_result_p
)
{
	HRESULT		retval			= E_FAIL;
	LONG			lock_count	= InterlockedIncrement(&m_lock_count);
	
	do
	{
		// �V���N���o�^����Ă��Ȃ��ꍇ�̓G���[
		if( m_sink_cp == NULL )
		{
			_RPTF0(_CRT_WARN, "*** Error *** m_sink_cp is NULL.\n");
			retval = E_UNEXPECTED;
			break;
		}
		
		// �L���Ȉ������H
		if( IsBadWritePtr(o_session_result_p, sizeof(HRESULT)) )
		{
			_RPTF0(_CRT_WARN, "*** Error *** o_session_result_p is bad ptr.\n");
			retval = E_INVALIDARG;
			break;
		}
		
		*o_session_result_p = E_FAIL;

		// ���b�N�Ɏ��s�����ꍇ�B
		if( lock_count != 1 )
		{
			_RPTF1(_CRT_WARN, "*** Error *** unable to lock. count : %d.\n", lock_count);

			// �������b�N��v���B
			if( (i_lock_flags & TS_LF_SYNC) == TS_LF_SYNC )
				*o_session_result_p = TS_E_SYNCHRONOUS;
			else	// �񓯊��v��
				*o_session_result_p = E_NOTIMPL;

			retval = S_OK;	// �߂�l�� S_OK
			break;
		}
		
		// OnLockGranted() ���R�[������B
		m_lock_flags = i_lock_flags;
		try
		{
			*o_session_result_p = m_sink_cp->OnLockGranted(i_lock_flags);
		}
		catch(...)
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** exception occured.\n");
			*o_session_result_p = E_FAIL;
		}
		m_lock_flags = 0;

		retval = S_OK;
	}
	while( 0 );
	
	InterlockedDecrement(&m_lock_count);
	
	return retval;
}


//=============================================================================


//! �h�L�������g�̃X�e�[�^�X���擾����B
STDMETHODIMP ReconvTextStore::GetStatus(
	TS_STATUS*	o_document_status_p
)
{
	HRESULT		retval = E_INVALIDARG;
	
	do
	{
		if( IsBadWritePtr(o_document_status_p, sizeof TS_STATUS) )
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** o_document_status_p is bad ptr.\n");
			retval = E_INVALIDARG;
			break;
		}
		
		o_document_status_p->dwDynamicFlags = TS_SD_READONLY;
		// ��̑I�����������Ȃ�
		o_document_status_p->dwStaticFlags = TS_SS_REGIONS;
		
		retval = S_OK;
	}
	while( 0 );

	return retval;
}


//=============================================================================


//! �h�L�������g�̑I��͈͂��擾����B
STDMETHODIMP ReconvTextStore::GetSelection(
	ULONG								i_index,
	ULONG								i_selection_buffer_length,
	TS_SELECTION_ACP*		o_selections,
	ULONG*							o_fetched_length_p
)
{
	HRESULT		retval = E_INVALIDARG;
	
	do
	{
		if( IsBadWritePtr(o_selections, sizeof TS_SELECTION_ACP)
		||	IsBadWritePtr(o_fetched_length_p, sizeof(ULONG)) )
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** invalid parameter.\n");
			retval = E_INVALIDARG;
			break;
		}
		
		*o_fetched_length_p = 0;
		
		if( IsLocked(TS_LF_READ) == false )
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** no lock.\n");
			retval = TS_E_NOLOCK;
			break;
		}
		
		if( i_index != TF_DEFAULT_SELECTION && i_index > 0 )
		{
			_RPTF0(_CRT_WARN, "*** ERROR *** i_index is not valid.\n");
			retval = E_INVALIDARG;
			break;
		}
		
		memset(o_selections, 0, sizeof(o_selections[0]));
		o_selections[0].acpStart = 0;
		if( m_text[0] )
			o_selections[0].acpEnd = lstrlen(m_text);
		else
			o_selections[0].acpEnd = 0;
		o_selections[0].style.fInterimChar	= FALSE;
		o_selections[0].style.ase						= TS_AE_START;

		*o_fetched_length_p = 1;
		retval = S_OK;
	}
	while( 0 );
	
	return retval;
}


//=============================================================================


//! �e�L�X�g���擾����B
STDMETHODIMP ReconvTextStore::GetText(
	LONG					i_start_index,
	LONG					i_end_index,
	WCHAR*				o_plain_text,
	ULONG					i_plain_text_length,
	ULONG*				o_plain_text_length_p,
	TS_RUNINFO*		o_run_info_p,
	ULONG					i_run_info_length,
	ULONG*				o_run_info_length_p,
	LONG*					o_next_unread_char_pos_p
)
{
	HRESULT		retval = E_INVALIDARG;
	
	do
	{
		if( IsLocked(TS_LF_READ) == false )
		{
			retval = TS_E_NOLOCK;
			break;
		}
		
		ULONG		text_length = lstrlen(m_text);
		ULONG		copy_length	= min(text_length, i_plain_text_length);
		
		// ��������i�[�B�R�s�[��� L'\0' �ŏI����Ă���K�v�͖����B
		if( IsBadWritePtr(o_plain_text, i_plain_text_length * sizeof(wchar_t)) == FALSE )
		{
			memset(o_plain_text, 0, i_plain_text_length * sizeof(wchar_t));
			memcpy(o_plain_text, m_text, copy_length * sizeof(wchar_t));
		}
		
		// �i�[����������̕��������i�[�B
		if( IsBadWritePtr(o_plain_text_length_p, sizeof ULONG) == FALSE )
		{
			*o_plain_text_length_p = copy_length;
		}
		
		// RUNINFO ���i�[�B
		if( IsBadWritePtr(o_run_info_p, sizeof TS_RUNINFO) == FALSE )
		{
			o_run_info_p[0].type   = TS_RT_PLAIN;
			o_run_info_p[0].uCount = text_length;
		}
		
		// RUNINFO ���i�[���������i�[�B
		if( IsBadWritePtr(o_run_info_length_p, sizeof ULONG) == FALSE )
			*o_run_info_length_p = 1;
		
		// ���̕����̈ʒu���i�[�B
		if( IsBadWritePtr(o_next_unread_char_pos_p, sizeof LONG) == FALSE )
			*o_next_unread_char_pos_p = i_start_index + text_length;
		
		retval = S_OK;
	}
	while( 0 );
	
	return retval;
}


//=============================================================================


//! �e�L�X�g�̃C���T�[�g�܂��͑I��͈͂̕ύX���\���₢���킹��B
STDMETHODIMP ReconvTextStore::QueryInsert(
	LONG		i_start_index,
	LONG		i_end_index,
	ULONG		i_length,
	LONG*		o_start_index_p,
	LONG*		o_end_index_p
)
{
	HRESULT	retval = E_FAIL;

	if( i_start_index < 0
	||	i_start_index > i_end_index
	||	i_end_index   > lstrlen(m_text) )
	{
		retval = E_INVALIDARG;
	}
	else
	{
		if( o_start_index_p )
			*o_start_index_p = i_start_index;
		
		if( o_end_index_p )
			*o_end_index_p   = i_end_index;

		retval = S_OK;
	}
	
	return retval;
}


//=============================================================================


STDMETHODIMP ReconvTextStore::GetActiveView(TsViewCookie*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::SetSelection(ULONG, const TS_SELECTION_ACP*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::SetText(DWORD, LONG, LONG, const WCHAR*, ULONG, TS_TEXTCHANGE*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetFormattedText(LONG, LONG, IDataObject**)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::RequestSupportedAttrs(DWORD, ULONG, const TS_ATTRID*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::RequestAttrsAtPosition(LONG, ULONG, const TS_ATTRID*, DWORD)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::RetrieveRequestedAttrs(ULONG, TS_ATTRVAL*, ULONG*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetEndACP(LONG*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetTextExt(TsViewCookie, LONG, LONG, RECT*, BOOL*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetScreenExt(TsViewCookie, RECT*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetWnd(TsViewCookie, HWND*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::InsertTextAtSelection(DWORD, const WCHAR*, ULONG, LONG*, LONG*, TS_TEXTCHANGE*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::RequestAttrsTransitioningAtPosition(LONG, ULONG, const TS_ATTRID*, DWORD)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::FindNextAttrTransition(LONG, LONG, ULONG, const TS_ATTRID*, DWORD, LONG*, BOOL*, LONG*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetEmbedded(LONG, REFGUID, REFIID, IUnknown**)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::QueryInsertEmbedded(const GUID*, const FORMATETC*, BOOL*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::InsertEmbedded(DWORD, LONG, LONG, IDataObject*, TS_TEXTCHANGE*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::GetACPFromPoint(TsViewCookie, const POINT*, DWORD, LONG*)
{	return E_NOTIMPL; }
STDMETHODIMP ReconvTextStore::InsertEmbeddedAtSelection(DWORD, IDataObject*, LONG*, LONG*, TS_TEXTCHANGE*)
{	return E_NOTIMPL; }


//=============================================================================


//! �R���X�g���N�^�BCreateInstance() ���T�u�N���X����̂݌Ăяo���\�B
ReconvTextStore::ReconvTextStore()
:	m_ref_count(1),
	m_sink_cp(NULL),
	m_sink_mask(0),
	m_lock_count(0),
	m_lock_flags(0)
{
	memset(m_text, 0, sizeof m_text);
}


//=============================================================================


