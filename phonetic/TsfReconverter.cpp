#include "stdafx.h"
#include <INITGUID.h>
#include "TsfReconverter.h"


//=============================================================================


//! �R���X�g���N�^
TsfReconverter::TsfReconverter()
:	m_thread_mgr_cp(NULL),
	m_document_mgr_cp(NULL),
	m_context_cp(NULL),
	m_function_provider_cp(NULL),
	m_reconversion_cp(NULL),
	m_text_store_cp(NULL),
	m_edit_cookie(0)
{
	HRESULT		hr = E_FAIL;
	
	do
	{
		// �X���b�h�}�l�[�W���[�̐���
		hr = CoCreateInstance(
					 CLSID_TF_ThreadMgr,
					 NULL,
					 CLSCTX_INPROC_SERVER,
					 IID_ITfThreadMgr,
					 (void**)&m_thread_mgr_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�X���b�h�}�l�[�W���[�̐����Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}
		
		// �h�L�������g�}�l�[�W���[�̐���
		hr = m_thread_mgr_cp->CreateDocumentMgr(&m_document_mgr_cp);
		if( FAILED(hr) || m_document_mgr_cp == NULL )
		{
			_RPTF1(_CRT_WARN,
						 "�h�L�������g�}�l�[�W���[�̐����Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}
		
		// �X���b�h�}�l�[�W���[�̃A�N�e�B�u��
		TfClientId	client_id = 0;
		hr = m_thread_mgr_cp->Activate(&client_id);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�X���b�h�}�l�[�W���[�̃A�N�e�B�u���Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}
		
		// �e�L�X�g�X�g�A�̐���
		ReconvTextStore::CreateInstance(m_text_store_cp);
		
		// �R���e�L�X�g�̐���
		hr = m_document_mgr_cp->CreateContext(
					 client_id,
					 0,			// reserved
					 (ITextStoreACP*)m_text_store_cp,
					 &m_context_cp,
					 &m_edit_cookie
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�R���e�L�X�g�̐����Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}
		
		// �R���e�L�X�g�� push
		hr = m_document_mgr_cp->Push(m_context_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�R���e�L�X�g�� push �Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}
		
		// �t�@���N�V�����v���o�C�_�[���擾����B
		hr = m_thread_mgr_cp->GetFunctionProvider(
					 GUID_SYSTEM_FUNCTIONPROVIDER,
					 &m_function_provider_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�t�@���N�V�����v���o�C�_�[�̎擾�Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}

		// ITfFnReconversion �̎擾
		hr = m_function_provider_cp->GetFunction(
					 GUID_NULL,
					 IID_ITfFnReconversion,
					 (IUnknown**)&m_reconversion_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN, "ITfFnReconversion �̎擾�Ɏ��s���܂����B�G���[�R�[�h : 0x%08X.\n", hr);
			break;
		}
		
		// �t�H�[�J�X�擾
		hr = m_thread_mgr_cp->SetFocus(m_document_mgr_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�X���b�h�}�l�[�W���[�� SetFocus() �Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
			break;
		}

		hr = S_OK;
	}
	while( 0 );
	
	if( FAILED(hr) )
		Cleanup();
	SetLastError(hr);
}


//=============================================================================


//! �f�X�g���N�^
TsfReconverter::~TsfReconverter()
{
	Cleanup();
}


//=============================================================================


//! �N���[���A�b�v
bool TsfReconverter::Cleanup()
{
	// �e�L�X�g�X�g�A�̉��
	RELEASE(m_text_store_cp);
	// ���R���o�[�W�����t�@���N�V�����̉��
	RELEASE(m_reconversion_cp);
	// �t�@���N�V�����v���o�C�_�[�̉��
	RELEASE(m_function_provider_cp);
	// �R���e�L�X�g�̉��
	RELEASE(m_context_cp);
	// �h�L�������g�}�l�[�W���[�̉��
	if( m_document_mgr_cp )
	{
		// �S�ẴR���e�L�X�g���������B
		m_document_mgr_cp->Pop(TF_POPF_ALL);
		RELEASE(m_document_mgr_cp);
	}
	// �X���b�h�}�l�[�W���[�̉��
	if( m_thread_mgr_cp )
	{
		// �f�A�N�e�B�u�ɂ��Ă���������B
		m_thread_mgr_cp->Deactivate();
		RELEASE(m_thread_mgr_cp);
	}
	
	return true;
}


//=============================================================================


//! �������ɐ����������ǂ������ׂ�B
bool TsfReconverter::IsOpened()
{
	return m_thread_mgr_cp != NULL;
}


//=============================================================================


//! �t�H�[�J�X�̎擾
bool TsfReconverter::SetFocus()
{
	bool		retval = false;
	
	if( IsOpened() )
	{
		// �t�H�[�J�X
		HRESULT hr = m_thread_mgr_cp->SetFocus(m_document_mgr_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "�X���b�h�}�l�[�W���[�� SetFocus() �Ɏ��s���܂����B�G���[�R�[�h:0x%08X\n", hr
						);
		}
		else
		{
			retval = true;
		}
	}
	
	return retval;
}


//=============================================================================


//! �ϊ���⃊�X�g��񋓂���B
bool TsfReconverter::EnumCandidates(
	const wchar_t*						i_yomi,
	EnumCandidatesCallbackP		i_callback,
	void*											io_param
)
{
	bool								retval					= false;
	HRESULT							hr							= E_FAIL;
	TF_SELECTION				selections[10]	= { 0 };
	ULONG								fetched_count		= 0;
	ITfRange*						range_cp				= NULL;
	BOOL								is_converted		= FALSE;
	ITfCandidateList*		cand_list_cp		= NULL;
	ULONG								cand_length			= 0;
	
	do
	{
		if( IsOpened() == false )
		{
			_RPTF0(_CRT_WARN, "�������Ɏ��s�����̂� EnumCandidates() ���Ă΂�܂����B\n");
			break;
		}
	
		// ��������Z�b�g
		m_text_store_cp->SetString(i_yomi);
		
		// �h�L�������g�̃��b�N���s���B
		if( m_text_store_cp->LockDocument(TS_LF_READ) )
		{
			// �I��͈͂̎擾
			hr = m_context_cp->GetSelection(
						 m_edit_cookie,
						 TF_DEFAULT_SELECTION,
						 numberof(selections),
						 selections,
						 &fetched_count
					 );
			
			// �h�L�������g�̃A�����b�N
			m_text_store_cp->UnlockDocument();
			
			if( FAILED(hr) )
			{
				_RPTF1(_CRT_WARN, "�I��͈͂̎擾�Ɏ��s���܂����B�G���[�R�[�h : 0x%08X.\n", hr);
				break;
			}
		}
		
		// �ϊ��͈͂��擾����B
		hr = m_reconversion_cp->QueryRange(selections[0].range, &range_cp, &is_converted);
		if( FAILED(hr) || range_cp == NULL )
		{
			_RPTF1(_CRT_WARN, "�ϊ��͈͂̎擾�Ɏ��s���܂����B�G���[�R�[�h : 0x%08X.\n", hr);
			break;
		}

		// �ĕϊ����s��
		hr = m_reconversion_cp->GetReconversion(selections[0].range, &cand_list_cp);
		if( FAILED(hr) || cand_list_cp == NULL )
		{
			_RPTF1(_CRT_WARN, "�ĕϊ��Ɏ��s���܂����B�G���[�R�[�h : 0x%08X.\n", hr);
			break;
		}
		
		// ��␔���擾����B
		hr = cand_list_cp->GetCandidateNum(&cand_length);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN, "��␔�̎擾�Ɏ��s���܂����B�G���[�R�[�h : 0x%08X.\n", hr);
			break;
		}
		
		// ����߂�l�ɃZ�b�g����B
		for(ULONG i = 0; i < cand_length; i++)
		{
			ITfCandidateString*		string_cp = NULL;
			BSTR									bstr			= NULL;

			if( SUCCEEDED(cand_list_cp->GetCandidate(i, &string_cp))
			&&	SUCCEEDED(string_cp->GetString(&bstr)) )
			{
				if( i_callback(i, cand_length, bstr, io_param) == false )
					i = cand_length;

				SysFreeString(bstr);
			}
			
			RELEASE(string_cp);
		}
		
		retval = true;
	}
	while( 0 );
	
	// �N���[���A�b�v
	RELEASE(cand_list_cp);
	RELEASE(range_cp);
	for(int i = 0; i < numberof(selections); i++)
	{
		RELEASE(selections[i].range);
	}
	
	return retval;
}


//=============================================================================