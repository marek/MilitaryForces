/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#include "../game/q_shared.h"
#include "qcommon.h"

static int			bloc = 0;

void	Huff_putBit( int bit, byte *fout, int *offset) {
	bloc = *offset;
	if ((bloc&7) == 0) {
		fout[(bloc>>3)] = 0;
	}
	fout[(bloc>>3)] |= bit << (bloc&7);
	bloc++;
	*offset = bloc;
}

int		Huff_getBloc()
{
	return bloc;
}

void	Huff_setBloc(int _bloc)
{
	bloc = _bloc;
}

int		Huff_getBit( byte *fin, int *offset) {
	int t;
	bloc = *offset;
	t = (fin[(bloc>>3)] >> (bloc&7)) & 0x1;
	bloc++;
	*offset = bloc;
	return t;
}

/* Add a bit to the output file (buffered) */
static void add_bit (char bit, byte *fout) {
	if ((bloc&7) == 0) {
		fout[(bloc>>3)] = 0;
	}
	fout[(bloc>>3)] |= bit << (bloc&7);
	bloc++;
}

/* Receive one bit from the input file (buffered) */
static int get_bit (byte *fin) {
	int t;
	t = (fin[(bloc>>3)] >> (bloc&7)) & 0x1;
	bloc++;
	return t;
}

node_t ** HuffmanTree::get_ppnode() {
	node_t **tppnode;
	if (!this->freelist) {
		return &(this->nodePtrs[this->blocPtrs++]);
	} else {
		tppnode = this->freelist;
		this->freelist = reinterpret_cast<node_t **>(*tppnode);
		return tppnode;
	}
}

void HuffmanTree::free_ppnode( node_t **ppnode ) {
	*ppnode = reinterpret_cast<node_t *>(this->freelist);
	this->freelist = ppnode;
}

/* Swap the location of these two nodes in the tree */
void HuffmanTree::swap( node_t *node1, node_t *node2 ) { 
	node_t *par1, *par2;

	par1 = node1->parent;
	par2 = node2->parent;

	if (par1) {
		if (par1->left == node1) {
			par1->left = node2;
		} else {
	      par1->right = node2;
		}
	} else {
		this->tree = node2;
	}

	if (par2) {
		if (par2->left == node2) {
			par2->left = node1;
		} else {
			par2->right = node1;
		}
	} else {
		this->tree = node1;
	}
  
	node1->parent = par2;
	node2->parent = par1;
}

/* Swap these two nodes in the linked list (update ranks) */
static void swaplist(node_t *node1, node_t *node2) {
	node_t *par1;

	par1 = node1->next;
	node1->next = node2->next;
	node2->next = par1;

	par1 = node1->prev;
	node1->prev = node2->prev;
	node2->prev = par1;

	if (node1->next == node1) {
		node1->next = node2;
	}
	if (node2->next == node2) {
		node2->next = node1;
	}
	if (node1->next) {
		node1->next->prev = node1;
	}
	if (node2->next) {
		node2->next->prev = node2;
	}
	if (node1->prev) {
		node1->prev->next = node1;
	}
	if (node2->prev) {
		node2->prev->next = node2;
	}
}

/* Do the increments */
void HuffmanTree::increment( node_t *node ) {
	node_t *lnode;

	if (!node) {
		return;
	}

	if (node->next != nullptr && node->next->weight == node->weight) {
	    lnode = *node->head;
		if (lnode != node->parent) {
			this->swap( lnode, node );
		}
		swaplist(lnode, node);
	}
	if (node->prev && node->prev->weight == node->weight) {
		*node->head = node->prev;
	} else {
	    *node->head = nullptr;
		this->free_ppnode( node->head );
	}
	node->weight++;
	if (node->next && node->next->weight == node->weight) {
		node->head = node->next->head;
	} else { 
		node->head = this->get_ppnode();
		*node->head = node;
	}
	if (node->parent) {
		this->increment( node->parent );
		if (node->prev == node->parent) {
			swaplist(node, node->parent);
			if (*node->head == node) {
				*node->head = node->parent;
			}
		}
	}
}

void HuffmanTree::addRef( byte ch ) {
	node_t *tnode, *tnode2;
	if (this->loc[ch] == nullptr) { /* if this is the first transmission of this node */
		tnode = &(this->nodeList[this->blocNode++]);
		tnode2 = &(this->nodeList[this->blocNode++]);

		tnode2->symbol = INTERNAL_NODE;
		tnode2->weight = 1;
		tnode2->next = this->lhead->next;
		if (this->lhead->next) {
			this->lhead->next->prev = tnode2;
			if (this->lhead->next->weight == 1) {
				tnode2->head = this->lhead->next->head;
			} else {
				tnode2->head = this->get_ppnode();
				*tnode2->head = tnode2;
			}
		} else {
			tnode2->head = this->get_ppnode();
			*tnode2->head = tnode2;
		}
		this->lhead->next = tnode2;
		tnode2->prev = this->lhead;
 
		tnode->symbol = ch;
		tnode->weight = 1;
		tnode->next = this->lhead->next;
		if (this->lhead->next) {
			this->lhead->next->prev = tnode;
			if (this->lhead->next->weight == 1) {
				tnode->head = this->lhead->next->head;
			} else {
				/* this should never happen */
				tnode->head = this->get_ppnode();
				*tnode->head = tnode2;
		    }
		} else {
			/* this should never happen */
			tnode->head = this->get_ppnode();
			*tnode->head = tnode;
		}
		this->lhead->next = tnode;
		tnode->prev = this->lhead;
		tnode->left = tnode->right = nullptr;
 
		if (this->lhead->parent) {
			if (this->lhead->parent->left == this->lhead) { /* lhead is guaranteed to by the NYT */
				this->lhead->parent->left = tnode2;
			} else {
				this->lhead->parent->right = tnode2;
			}
		} else {
			this->tree = tnode2; 
		}
 
		tnode2->right = tnode;
		tnode2->left = this->lhead;
 
		tnode2->parent = this->lhead->parent;
		this->lhead->parent = tnode->parent = tnode2;
     
		this->loc[ch] = tnode;
 
		this->increment( tnode2->parent );
	} else {
		this->increment( this->loc[ch] );
	}
}

/* Get a symbol */
int Huff_Receive (node_t *node, int *ch, byte *fin) {
	while (node && node->symbol == INTERNAL_NODE) {
		if (get_bit(fin)) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	if (!node) {
		return 0;
//		Common::error(ERR_DROP, "Illegal tree!");
	}
	return (*ch = node->symbol);
}

/* Get a symbol */
void Huff_offsetReceive (node_t *node, int *ch, byte *fin, int *offset, int maxoffset) {
	bloc = *offset;
	while (node && node->symbol == INTERNAL_NODE) {
		if (bloc >= maxoffset) {
			*ch = 0;
			*offset = maxoffset + 1;
			return;
		}
		if (get_bit(fin)) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	if (!node) {
		*ch = 0;
		return;
//		Common::error(ERR_DROP, "Illegal tree!");
	}
	*ch = node->symbol;
	*offset = bloc;
}

/* Send the prefix code for this node */
static void send(node_t *node, node_t *child, byte *fout, int maxoffset) {
	if (node->parent) {
		send(node->parent, node, fout, maxoffset);
	}
	if (child) {
		if (bloc >= maxoffset) {
			bloc = maxoffset + 1;
			return;
		}
		if (node->right == child) {
			add_bit(1, fout);
		} else {
			add_bit(0, fout);
		}
	}
}

/* Send a symbol */
void HuffmanTree::transmit( int ch, byte *fout, int maxoffset ) {
	int i;
	if (this->loc[ch] == nullptr) { 
		/* node_t hasn't been transmitted, send a NYT, then the symbol */
		this->transmit( NYT, fout, maxoffset );
		for (i = 7; i >= 0; i--) {
			add_bit(static_cast<char>((ch >> i) & 0x1), fout);
		}
	} else {
		send(this->loc[ch], nullptr, fout, maxoffset);
	}
}

void HuffmanTree::offsetTransmit( int ch, byte *fout, int *offset, int maxoffset ) {
	bloc = *offset;
	send(this->loc[ch], nullptr, fout, maxoffset);
	*offset = bloc;
}

void Huff_Decompress(Message *mbuf, int offset) {
	int			ch, cch, i, j, size;
	byte		seq[65536];
	byte*		buffer;
	HuffmanTree		huff;

	size = mbuf->size() - offset;
	buffer = mbuf->data() + offset;

	if ( size <= 0 ) {
		return;
	}

	Com_Memset(&huff, 0, sizeof(HuffmanTree));
	// Initialize the tree & list with the NYT node 
	huff.tree = huff.lhead = huff.ltail = huff.loc[NYT] = &(huff.nodeList[huff.blocNode++]);
	huff.tree->symbol = NYT;
	huff.tree->weight = 0;
	huff.lhead->next = huff.lhead->prev = nullptr;
	huff.tree->parent = huff.tree->left = huff.tree->right = nullptr;

	cch = buffer[0]*256 + buffer[1];
	// don't overflow with bad messages
	if ( cch > mbuf->maxSize() - offset ) {
		cch = mbuf->maxSize() - offset;
	}
	bloc = 16;

	for ( j = 0; j < cch; j++ ) {
		ch = 0;
		// don't overflow reading from the messages
		// FIXME: would it be better to have an overflow check in get_bit ?
		if ( (bloc >> 3) > size ) {
			seq[j] = 0;
			break;
		}
		Huff_Receive(huff.tree, &ch, buffer);				/* Get a character */
		if ( ch == NYT ) {								/* We got a NYT, get the symbol associated with it */
			ch = 0;
			for ( i = 0; i < 8; i++ ) {
				ch = (ch<<1) + get_bit(buffer);
			}
		}
    
		seq[j] = ch;									/* Write symbol */

		huff.addRef( static_cast<byte>(ch) );								/* Increment node */
	}
	mbuf->setSize( cch + offset );
	Com_Memcpy(mbuf->data() + offset, seq, cch);
}

extern 	int oldsize;

void Huff_Compress(Message *mbuf, int offset) {
	int			i, ch, size;
	byte		seq[65536];
	byte*		buffer;
	HuffmanTree		huff;

	size = mbuf->size() - offset;
	buffer = mbuf->data() + offset;

	if (size<=0 || size > 65535) {
		return;
	}

	Com_Memset(&huff, 0, sizeof(HuffmanTree));
	// Add the NYT (not yet transmitted) node into the tree/list */
	huff.tree = huff.lhead = huff.loc[NYT] =  &(huff.nodeList[huff.blocNode++]);
	huff.tree->symbol = NYT;
	huff.tree->weight = 0;
	huff.lhead->next = huff.lhead->prev = nullptr;
	huff.tree->parent = huff.tree->left = huff.tree->right = nullptr;

	seq[0] = (size>>8);
	seq[1] = size&0xff;

	bloc = 16;

	for (i=0; i<size; i++ ) {
		ch = buffer[i];
		huff.transmit( ch, seq, size<<3 );						/* Transmit symbol */
		huff.addRef( static_cast<byte>(ch) );								/* Do update */
	}

	bloc += 8;												// next byte

	mbuf->setSize( (bloc>>3) + offset );
	Com_Memcpy(mbuf->data()+offset, seq, (bloc>>3));
}

void Huff_Init(huffman_t *huff) {

	Com_Memset(&huff->compressor, 0, sizeof(HuffmanTree));
	Com_Memset(&huff->decompressor, 0, sizeof(HuffmanTree));

	// Initialize the tree & list with the NYT node 
	huff->decompressor.tree = huff->decompressor.lhead = huff->decompressor.ltail = huff->decompressor.loc[NYT] = &(huff->decompressor.nodeList[huff->decompressor.blocNode++]);
	huff->decompressor.tree->symbol = NYT;
	huff->decompressor.tree->weight = 0;
	huff->decompressor.lhead->next = huff->decompressor.lhead->prev = nullptr;
	huff->decompressor.tree->parent = huff->decompressor.tree->left = huff->decompressor.tree->right = nullptr;

	// Add the NYT (not yet transmitted) node into the tree/list */
	huff->compressor.tree = huff->compressor.lhead = huff->compressor.loc[NYT] =  &(huff->compressor.nodeList[huff->compressor.blocNode++]);
	huff->compressor.tree->symbol = NYT;
	huff->compressor.tree->weight = 0;
	huff->compressor.lhead->next = huff->compressor.lhead->prev = nullptr;
	huff->compressor.tree->parent = huff->compressor.tree->left = huff->compressor.tree->right = nullptr;
}

