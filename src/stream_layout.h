/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef STREAM_LAYOUT_T_H
#define STREAM_LAYOUT_T_H

#include <string>

#include "tree.hh"

#include "ace/Global_Macros.h"
#include "ace/Stream.h"

#include "common_idumpstate.h"

#include "stream_common.h"
//#include "stream_istreamcontrol.h"

// *IMPORTANT NOTE*: the current implementation uses an n-ary tree ADT as model;
//                   this has the following implications:
//                   - 'head' sibling nodes (i.e. depth 0) are linked and form
//                     the 'main' branch
//                   - distributor modules are always last (!) siblings of the
//                     processing (sub-)stream at any specific depth (unless
//                     they are direct children (!) of distributors themselves)
//                     and terminated by the stream tail.
//                     Each of their direct children form a separate branch
//                     defined by the direct child and the direct childs'
//                     children siblings, iff the direct child is not (!) a
//                     distributor itself; otherwise the branch definition(s)
//                     simply recurse one level 'deeper'
//                   - thus, (consecutive) 'leaf' nodes at any specific depth
//                     represent trailing sub-streams
//                   --> to setup the stream:
//                       a) iterate breadth-first link()ing the ('head') modules
//                       (use a 'sibling' iterator).
//                       b) iff the final sibling is a distributor, append the
//                       stream tail and proceed depth-first one level (i.e. use
//                       another 'sibling' iterator on the distributors'
//                       children)
//                       b1) each distributors' non-distributor (!) direct (!)
//                       child represents the 'head' module of a distinct
//                       processing branch; use a 'sibling' iterator on its'
//                       children (iff any) and resume link()ing (i.e. goto a),
//                       orr simply append the stream tail and resume)
//                       d) iff a distributors' direct child is itself a
//                       distributor goto b) (i.e. append stream tail and
//                       descend)
//                   --> to unwind the stream:
//                       a) use a 'post-order' iterator to retrieve the 'head'
//                       leaf. Save the current depth and current module (A)
//                       b) iff there is a parent node module (C), and (!) it is
//                       link()ed to A, As' sibling leaves form a (trailing)
//                       (sub-)stream (see above)
//                       b1) Continue queueing modules until the iteration
//                       ascends to C (depth change) or end()s (end of 'main'
//                       branch)
//                       Prepend C (if any) and unlink() back-to-front. Unlink()
//                       C from its' parent distributor and restart
//                       c) else (iff there is a parent module) C is a
//                       distributor and A is either:
//                       c1) a non-distributor leaf node with a parent
//                       distributor; unlink() A, remove from C and restart
//                       c2) or a leaf (empty) distributor; proceed as c1)
//                       d) A has no parent module; unwind the 'main' branch

// forward declarations
class Stream_IDistributorModule;
template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_IStream_T;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename DistributorModuleType>
class Stream_Layout_T
 : public tree<ACE_Module<ACE_SYNCH_USE,
                          TimePolicyType>*,
               std::allocator<tree_node_<ACE_Module<ACE_SYNCH_USE,
                                                    TimePolicyType>*> > >
 , public Common_IDumpState
{
  typedef tree<ACE_Module<ACE_SYNCH_USE,
                          TimePolicyType>*,
               std::allocator<tree_node_<ACE_Module<ACE_SYNCH_USE,
                                                    TimePolicyType>*> > > inherited;

 public:
  // convenient types
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef typename inherited::iterator ITERATOR_T;

  Stream_Layout_T ();
  inline virtual ~Stream_Layout_T () {}

  bool setup (STREAM_T&);
  void unset (STREAM_T&);

  MODULE_T* find (const std::string&,  // nodule name
                  bool = false) const; // sanitize module names ?
  Stream_ModuleList_t prev (const std::string&) const; // nodule name
  Stream_ModuleList_t next (const std::string&) const; // nodule name

  // append a module to a branch
  bool append (MODULE_T*,         // module handle
               MODULE_T* = NULL,  // distributor module handle {NULL: 'main' branch}
               unsigned int = 0); // distributor sub-branch index, if any (zero-based)
  bool append (MODULE_T*,           // module handle
               const std::string&); // branch name {"": 'main' branch}
  bool remove (const std::string&); // nodule name

  // *NOTE*: returns the layout sorted depth-first (aka 'pre-order', or
  //         'element-before-children')
  Stream_ModuleList_t list (bool = false) const; // 'main' branch only ?

  virtual void dump_state () const;

 private:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef typename inherited::iterator_base BASE_ITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Layout_T (const Stream_Layout_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Layout_T& operator= (const Stream_Layout_T&))

  // helper methods
  bool setup (typename inherited::tree_node&, // distributor node
              ISTREAM_T*, // stream handle
              MODULE_T*); // stream tail handle

  bool find (MODULE_T*,
             typename inherited::iterator_base&) const;
  void prev (typename inherited::tree_node&,                  // distributor node
             const std::string&,          // module name
             Stream_ModuleList_t&) const; // return value

  inline bool is_distributor (MODULE_T* module_in) const { return dynamic_cast<Stream_IDistributorModule*> (module_in->writer ()); }
  // *NOTE*: the return index value is correct as long as:
  //         - the module has been initialize()d
  //         - the corresponding head module has not been push()ed yet
  bool has_branch (typename inherited::tree_node&,           // distributor node
                   const std::string&,   // branch name
                   unsigned int&) const; // return value: index (iff any; see above)
};

// include template definition
#include "stream_layout.inl"

#endif
