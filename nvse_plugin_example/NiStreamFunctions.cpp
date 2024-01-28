
#include "NiNodes.h"

NiStream* NiStream::Create(NiStream* apThis) {
	return ThisStdCall<NiStream*>(0xA66150, apThis);
}

// 0xA66370
void NiStream::InsertObject(NiNode* apObject) {
	ThisStdCall(0xA66370, this, apObject);
}
