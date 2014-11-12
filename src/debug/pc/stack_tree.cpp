#include "debug/stack_tree.h"
#include "core/lumix.h"
#include "core/MT/atomic.h"
#include <Windows.h>
#include <cstdio>
#include <Dbghelp.h>


#pragma comment(lib, "Dbghelp.lib")


namespace Lumix
{


namespace Debug
{

	class StackNode
	{
		public:
			~StackNode()
			{
				delete m_next;
				delete m_first_child;
			}

			void* m_instruction;
			StackNode* m_next;
			StackNode* m_first_child;
			StackNode* m_parent;
	};


	StackTree::StackTree()
	{
		m_root = NULL;
		if(MT::atomicIncrement(&m_instances) == 1)
		{
			HANDLE process = GetCurrentProcess();
			SymInitialize(process, NULL, TRUE);
		}
	}


	StackTree::~StackTree()
	{
		delete m_root;
		if(MT::atomicDecrement(&m_instances) == 0)
		{
			HANDLE process = GetCurrentProcess();
			SymCleanup(process);
		}
	}


	void StackTree::printCallstack(StackNode* node)
	{
		while(node)
		{
			HANDLE process = GetCurrentProcess();
			uint8_t symbol_mem[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
			SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_mem);
			memset(symbol_mem, 0, sizeof(symbol_mem));
			symbol->MaxNameLen = 255;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			SymFromAddr(process, (DWORD64)(node->m_instruction), 0, symbol);
			OutputDebugString("\t");
			OutputDebugString(symbol->Name);
			OutputDebugString("\n");
			node = node->m_parent;
		}
	}


	StackNode* StackTree::insertChildren(StackNode* root_node, void** instruction, void** stack)
	{
		StackNode* node = root_node;
		while(instruction >= stack)
		{
			StackNode* new_node = new StackNode();
			node->m_first_child = new_node;
			new_node->m_parent = node;
			new_node->m_next = NULL;
			new_node->m_first_child = NULL;
			new_node->m_instruction = *instruction;
			node = new_node;
			--instruction;
		}
		return node;
	}

	
	StackNode* StackTree::record()
	{
		static const int frames_to_capture = 256;
		void* stack[frames_to_capture];
		USHORT captured_frames_count = CaptureStackBackTrace(2, frames_to_capture, stack, 0);
		
		void** ptr = stack + captured_frames_count - 1;
		if(!m_root)
		{
			m_root = new StackNode();
			m_root->m_instruction = *ptr;
			m_root->m_first_child = NULL;
			m_root->m_next = NULL;
			m_root->m_parent = NULL;
			--ptr;
			return insertChildren(m_root, ptr, stack);
		}

		StackNode* node = m_root;
		while(ptr >= stack)
		{
			while(node->m_instruction != *ptr && node->m_next)
			{
				node = node->m_next;
			}
			if(node->m_instruction != *ptr)
			{
				node->m_next = new StackNode;
				node->m_next->m_parent = node->m_parent;
				node->m_next->m_instruction = *ptr;
				node->m_next->m_next = NULL;
				node->m_next->m_first_child = NULL;
				--ptr;
				return insertChildren(node, ptr, stack);
			}
			else if(node->m_first_child)
			{
				--ptr;
				node = node->m_first_child;
			}
			else if(ptr != stack)
			{
				node->m_first_child = new StackNode;
				node->m_first_child->m_parent = node;
				node->m_next = NULL;
				node->m_first_child = NULL;
				node->m_instruction = *ptr;
				--ptr;
				return insertChildren(node, ptr, stack);
			}
			else
			{
				return node;
			}
		}

		ASSERT(false);
		return NULL;		
	}


} // namespace Debug


} // namespace Lumix