#ifndef __TSF_RECONVERTER_H_INCLUDED__
#define __TSF_RECONVERTER_H_INCLUDED__
#include <msctf.h>
#include <msaatext.h>
#include <ctffunc.h>
#include "ReconvTextStore.h"


//! TsfReconverter::EnumCandidates() �ɓn���R�[���o�b�N�֐�
/*!
 *	@param[in]			i_index
 *		���Ԗڂ̌�₩�B
 *	@param[in]			i_number_of_candidates
 *		���̃g�[�^����
 *	@param[in]			i_candidate
 *		��╶����B�R�[���o�b�N���甲�����當����͉�������B
 *	@param[in,out]	io_param
 *		EnumCandidates() �ɓn�����p�����[�^�[�B
 *
 *	@return �񋓂𑱂���ꍇ�� true, ���~����ꍇ�� false ��Ԃ��B
 */
typedef bool (*EnumCandidatesCallbackP)(int							i_index,
																				int							i_number_of_candidates,
																				const wchar_t*	i_candidate,
																				void*						io_param
																			 );


//! TSF(Text Service Framework) �𗘗p���ĕϊ���⃊�X�g���擾����N���X�B
class TsfReconverter
{
	//===========================================================================
	// ���J���\�b�h
	//===========================================================================
public:
	//! �R���X�g���N�^
	TsfReconverter();
	//! �f�X�g���N�^
	virtual ~TsfReconverter();
	
	//! �N���[���A�b�v
	bool Cleanup();
	
	//! �������ɐ����������ǂ������ׂ�B
	bool IsOpened();
	
	//! �t�H�[�J�X�̎擾�B
	bool SetFocus();

	//! �ϊ���⃊�X�g��񋓂���B
	bool EnumCandidates(
		const wchar_t*						i_yomi,
		EnumCandidatesCallbackP		i_callback,
		void*											io_param
	);
	
	//===========================================================================
	// ����J���\�b�h
	//===========================================================================
private:

	//! �R�s�[�R���X�g���N�^�͎g�p�֎~
	TsfReconverter(const TsfReconverter&);
	//! ������Z�q�͎g�p�֎~
	const TsfReconverter& operator=(const TsfReconverter&);

	//===========================================================================
	// �����o
	//===========================================================================
public:
	ITfThreadMgr*					m_thread_mgr_cp;
	ITfDocumentMgr*				m_document_mgr_cp;
	ITfContext*						m_context_cp;
	ITfFunctionProvider*	m_function_provider_cp;
	ReconvTextStore*			m_text_store_cp;
	ITfFnReconversion*		m_reconversion_cp;
	TfEditCookie					m_edit_cookie;
};


#endif