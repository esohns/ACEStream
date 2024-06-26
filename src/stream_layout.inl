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

#include <vector>

#include "tree_util.hh"

#include "ace/Log_Msg.h"
#include "ace/Module.h"
#include "ace/OS.h"

#include "common_macros.h"

#include "stream_defines.h"
#include "stream_ilink.h"
#include "stream_macros.h"
#include "stream_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::Stream_Layout_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::Stream_Layout_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::setup (STREAM_T& stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::setup"));

  int result = -1;
  Stream_ModuleList_t main_branch_a;
  std::vector<typename inherited::tree_node*> distributors_a;

  // step1: reset stream
  result = stream_in.close (0); // <-- retain head/tail modules
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(0): \"%m\", aborting\n")));
    return false;
  } // end IF

  // step2: set up 'main' branch
  // *IMPORTANT NOTE*: ACE_Stream modules must be push()ed back-to-front
  //                   --> extract 'main' branch first
  for (typename inherited::fixed_depth_iterator iterator = inherited::begin_fixed (inherited::begin (), 0, false);
//       iterator != inherited::end_fixed (inherited::begin (), 0);
       inherited::is_valid (iterator);
       ++iterator)
  {
    main_branch_a.push_back (*iterator);
    if (unlikely (is_distributor (*iterator)))
      distributors_a.push_back (iterator.node);
  } // end FOR
  for (Stream_ModuleListReverseIterator_t iterator = main_branch_a.rbegin ();
       iterator != main_branch_a.rend ();
       ++iterator)
  { ACE_ASSERT (*iterator);
    result = stream_in.push (*iterator);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                  (*iterator)->name ()));
      goto error;
    } // end IF
  } // end FOR

  // step3: set up any sub-branches
  for (typename std::vector<typename inherited::tree_node*>::const_iterator iterator = distributors_a.begin ();
       iterator != distributors_a.end ();
       ++iterator)
    if (unlikely (!setup (*(*iterator))))
      goto error;

  return true;

error:
  result = stream_in.close (0);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(0): \"%m\", aborting\n")));

  inherited::clear ();

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
void
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::unset (STREAM_T& stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::unset"));

  int result = -1;
  int depth_i = 0;
  MODULE_T* module_p = NULL, module_2 = NULL;
  Stream_ModuleList_t modules_a;
  typename inherited::post_order_iterator iterator, iterator_2;
  Stream_IDistributorModule* idistributor_p = NULL;

restart:
  modules_a.clear ();
  iterator = inherited::begin_post ();
  depth_i = inherited::depth (iterator);
  module_p = *iterator;
  iterator_2 = inherited::parent (iterator);
  if (iterator_2 == inherited::end ()) // || depth_i == 0
    goto end; // --> only 'main' branch left (d))
  while (inherited::depth (iterator) == depth_i)
  {
    modules_a.push_back (*iterator);
    ++iterator;
  } // end WHILE
  if (!ACE_OS::strcmp ((*iterator_2)->next ()->name (),
                       module_p->name ()))
  { // C --> A (b))
    modules_a.push_front (*iterator_2);

    iterator = inherited::parent (iterator_2);
    ACE_ASSERT (inherited::is_valid (iterator));
    idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> ((*iterator)->writer ());
    ACE_ASSERT (idistributor_p); // C has a distributor parent
    idistributor_p->pop (*iterator_2);
  } // end IF
  else
  { // C -x> A, i.e. C is a distributor
    idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> ((*iterator_2)->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->pop (module_p);
#if defined (_DEBUG)
    idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    if (!idistributor_p)
    { // A is a non-distributor leaf (c1)
      ACE_ASSERT (modules_a.size () == 1);
    } // end IF
    else
    { // A is a leaf (empty) distributor (c2)
      ACE_ASSERT (idistributor_p->next ().empty ());
      ACE_ASSERT (modules_a.size () == 1);
    } // end ELSE
#endif // _DEBUG
  } // end ELSE
  for (Stream_ModuleListReverseIterator_t iterator_3 = modules_a.rbegin ();
       iterator_3 != modules_a.rend ();
       ++iterator_3)
  {
    /*result =*/ (*iterator_3)->link (NULL);
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Module::link(NULL): \"%m\", continuing\n")));
  } // end FOR
  goto restart;

end:
  // unwind 'main' branch
  result = stream_in.close (STREAM_T::M_DELETE);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(%d): \"%m\", continuing\n"),
                STREAM_T::M_DELETE));

  inherited::clear ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
ACE_Module<ACE_SYNCH_USE, TimePolicyType>*
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::find (const std::string& name_in,
                                           bool sanitizeModuleNames_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::find"));

  std::string module_name_string =
      (sanitizeModuleNames_in ? Stream_Tools::sanitizeUniqueName (name_in)
                              : name_in);

  std::string module_name_string_2;
  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
  {
    module_name_string_2 =
        (sanitizeModuleNames_in ? Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR ((*iterator)->name ()))
                                : ACE_TEXT_ALWAYS_CHAR ((*iterator)->name ()));
    if (unlikely (!ACE_OS::strcmp (module_name_string_2.c_str (),
                                   name_in.c_str ())))
      return (*iterator);
  } // end FOR

  return NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
Stream_ModuleList_t
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::prev (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::prev"));

  // initialize return value(s)
  Stream_ModuleList_t return_value;

  MODULE_T* prev_p = NULL;
  for (typename inherited::fixed_depth_iterator iterator = inherited::begin_fixed (inherited::begin (), 0, false);
       inherited::is_valid (iterator);
//       iterator != inherited::end_fixed (inherited::begin (), 0);
       ++iterator)
  {
    if (unlikely (inherited::number_of_children (iterator)))
      prev (*(iterator.node),
            name_in,
            return_value);

    if (unlikely (!ACE_OS::strcmp ((*iterator)->name (),
                                   ACE_TEXT (name_in.c_str ())) &&
                  prev_p))
      return_value.push_back (prev_p);
    prev_p = *iterator;
  } // end FOR

  return return_value;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
Stream_ModuleList_t
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::next (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::next"));

  // initialize return value(s)
  Stream_ModuleList_t return_value;

  MODULE_T* module_p = NULL;
  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
  {
    if (likely (ACE_OS::strcmp ((*iterator)->name (),
                                ACE_TEXT (name_in.c_str ()))))
      continue;

    module_p = (*iterator)->next ();
    if (unlikely (!module_p))
    { // --> module may be a distributor
      Stream_IDistributorModule* inext_p =
          dynamic_cast<Stream_IDistributorModule*> ((*iterator)->writer ());
      if (!inext_p)
        break; // --> module is an aggregator
      return_value = inext_p->next ();
      break;
    } // end IF
    if (unlikely (ACE_OS::strcmp (module_p->name (),
                                  ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
                  ACE_OS::strcmp (module_p->name (),
                                  ACE_TEXT ("ACE_Stream_Tail"))))
      return_value.push_back (module_p); // --> module is not the tail
    break;
  } // end FOR

  return return_value;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::append (MODULE_T* module_in,
                                             MODULE_T* distributorModule_in,
                                             unsigned int index_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::append"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  if (unlikely (inherited::empty ()))
  { ACE_ASSERT (!distributorModule_in);
    typename inherited::pre_order_iterator iterator =
        inherited::set_head (module_in);
    return inherited::is_valid (iterator);
  } // end IF

  typename inherited::sibling_iterator iterator;
  if (unlikely (distributorModule_in))
  {
    // establish branch head
    find (distributorModule_in, iterator);
    ACE_ASSERT (inherited::is_valid (iterator));
    unsigned int num_existing_branches_i =
        inherited::number_of_children (iterator);
    if (unlikely (!num_existing_branches_i ||
                  ((num_existing_branches_i - 1) < index_in)))
    { // --> module is (new) branch 'head'
      iterator = inherited::append_child (iterator, module_in);
      return inherited::is_valid (iterator);
    } // end IF
    ACE_ASSERT (num_existing_branches_i && (index_in <= (num_existing_branches_i - 1)));
    // --> append to existing branch
    iterator = iterator.begin ();
    ACE_ASSERT (inherited::is_valid (iterator));
    iterator = inherited::sibling (iterator, index_in);
    ACE_ASSERT (inherited::is_valid (iterator));
    iterator = inherited::append_child (iterator, module_in);
    return inherited::is_valid (iterator);
  } // end IF

  typename inherited::sibling_iterator iterator_end = inherited::end ();
  // --> append to 'main' branch
  iterator = inherited::begin ();
  ACE_ASSERT (inherited::is_valid (iterator));
  // *TODO*: try --iterator_end
  while (inherited::next_sibling (iterator) != iterator_end)
    ++iterator;
  ACE_ASSERT (inherited::is_valid (iterator));
  iterator = inherited::insert_after (iterator, module_in);

  return (inherited::is_valid (iterator));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::append (MODULE_T* module_in,
                                             const std::string& branchName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::append"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  if (unlikely (branchName_in.empty ()))
    return append (module_in, NULL, 0);

  unsigned int index_i = 0;
  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
  {
    if (likely (!is_distributor (*iterator)) ||
                !has_branch (*(iterator.node),
                             branchName_in,
                             index_i))
      continue;
    return append (module_in, *iterator, index_i);
  } // end FOR
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("branch (was: \"%s\") not found, aborting\n"),
              ACE_TEXT (branchName_in.c_str ())));

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::remove (const std::string& name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::remove"));

  bool result = false;

restart:
  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
  {
    if (unlikely (!ACE_OS::strcmp ((*iterator)->name (),
                                   ACE_TEXT (name_in.c_str ()))))
    {
      inherited::erase (iterator);
      result = true;
      goto restart;
    } // end IF
  } // end FOR

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::replace (const std::string& name_in,
                                              MODULE_T* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::replace"));

  bool result = false;

  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
  {
    if (unlikely (!ACE_OS::strcmp ((*iterator)->name (),
                                   ACE_TEXT (name_in.c_str ()))))
    {
      iterator.node->data = module_in;
      result = true;
      break;
    } // end IF
  } // end FOR

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
Stream_ModuleList_t
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::list (bool mainBranchOnly_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::list"));

  // initialize return value(s)
  Stream_ModuleList_t return_value;

  if (unlikely (mainBranchOnly_in))
  {
    for (typename inherited::fixed_depth_iterator iterator = inherited::begin_fixed (inherited::begin (), 0, false);
         inherited::is_valid (iterator);
//         iterator != inherited::end_fixed (inherited::begin (), 0);
         ++iterator)
      return_value.push_back (*iterator);
    return return_value;
  } // end IF

  for (typename inherited::iterator iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
    return_value.push_back (*iterator);

  return return_value;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
void
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::dump_state"));

//  kptree::print_tree_bracketed (*this, std::cout);

  size_t depth_i = 0, count_i = 0, num_nodes_i = inherited::size ();
  ACE_UNUSED_ARG (num_nodes_i);
  std::string indentation_string, tree_layout_string;
  bool is_last_b = false;
  typename inherited::fixed_depth_iterator iterator_s;
//  for (typename inherited::breadth_first_iterator iterator = inherited::begin_breadth_first ();
//       iterator != inherited::end_breadth_first ();
//       ++iterator, ++count_i)
  int max_depth_i = inherited::max_depth ();
  for (int i = 0;
       i <= max_depth_i;
       ++i)
    for (typename inherited::fixed_depth_iterator iterator = inherited::begin_fixed (inherited::begin (), i, false);
         inherited::is_valid (iterator);
//         iterator != inherited::end_fixed (inherited::begin (), i);
         ++iterator, ++count_i)
    {
      tree_layout_string +=
          ((depth_i < static_cast<size_t> (inherited::depth (iterator))) ? ACE_TEXT_ALWAYS_CHAR ("\n")
                                                                         : ACE_TEXT_ALWAYS_CHAR (""));
      indentation_string.insert (0, inherited::depth (iterator), '\t');
      tree_layout_string +=
          ((inherited::depth (iterator) ? ((*iterator == iterator.node->parent->first_child->data) ? indentation_string.c_str ()
                                                                                                   : ACE_TEXT_ALWAYS_CHAR (""))
                                        : ACE_TEXT_ALWAYS_CHAR ("")));
      indentation_string.clear ();
      tree_layout_string += ACE_TEXT_ALWAYS_CHAR ((*iterator)->name ());
      iterator_s = iterator; ++iterator_s;
      is_last_b =
          (inherited::depth (iterator) ? (*iterator == iterator.node->parent->last_child->data)
                                       : !inherited::is_valid (iterator_s));
      tree_layout_string +=
          (is_last_b ? ((i == 0) ? ACE_TEXT_ALWAYS_CHAR ("")
                                 : ((i % 2) ? ACE_TEXT_ALWAYS_CHAR ("")
                                            : ACE_TEXT_ALWAYS_CHAR ("\n")))
                     : ((i % 2) ? ACE_TEXT_ALWAYS_CHAR (" | ")
                                : ACE_TEXT_ALWAYS_CHAR (" --> ")));

      depth_i = static_cast<size_t> (inherited::depth (iterator));
    } // end FOR
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s"),
              ACE_TEXT (tree_layout_string.c_str ())));
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::setup (NODE_T& node_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::setup"));

  // sanity check(s)
  typename inherited::iterator_base base_iterator (&node_in);
  ACE_ASSERT (is_distributor (node_in.data));

  Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (node_in.data->writer ());
  ACE_ASSERT (idistributor_p);

  int result = -1;
  TASK_T* task_p = NULL;
  MODULE_T* prev_p = NULL, *tail_p = NULL;
  std::vector<typename inherited::iterator_base> sub_distributors_a;
  for (typename inherited::sibling_iterator iterator = inherited::begin (&node_in);
       iterator != inherited::end (&node_in);
       ++iterator)
  {
    task_p = (*iterator)->reader ();
    ACE_ASSERT (task_p);
    result = task_p->open ((*iterator)->arg ());
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_Base::open (): \"%m\", aborting\n"),
                  (*iterator)->name ()));
      return false;
    } // end IF
    task_p = (*iterator)->writer ();
    ACE_ASSERT (task_p);
    result = task_p->open ((*iterator)->arg ());
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_Base::open (): \"%m\", aborting\n"),
                  (*iterator)->name ()));
      return false;
    } // end IF

    // associate all direct children to the distributor
    idistributor_p->push (*iterator);

    // link sub-stream head to its' distributor
    // *WARNING*: link reader-side only !
    //node_in.data->link (*iterator);
    (*iterator)->reader ()->next (node_in.data->reader ());

    // link sub-branch, retain any sub-distributors
    prev_p = *iterator;
    for (typename inherited::sibling_iterator iterator_2 = inherited::begin (iterator);
         iterator_2 != inherited::end (iterator);
         ++iterator_2)
    {
      task_p = (*iterator_2)->reader ();
      ACE_ASSERT (task_p);
      result = task_p->open ((*iterator_2)->arg ());
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_Base::open (): \"%m\", aborting\n"),
                    (*iterator_2)->name ()));
        return false;
      } // end IF
      task_p = (*iterator_2)->writer ();
      ACE_ASSERT (task_p);
      result = task_p->open ((*iterator_2)->arg ());
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_Base::open (): \"%m\", aborting\n"),
                    (*iterator_2)->name ()));
        return false;
      } // end IF
      
      ACE_ASSERT (prev_p);
      prev_p->link (*iterator_2);
      prev_p = *iterator_2;

      if (unlikely (is_distributor (*iterator_2)))
        sub_distributors_a.push_back (iterator_2);
    } // end FOR
    ACE_ASSERT (prev_p);

    tail_p = makeSubStreamTail ();
    if (unlikely (!tail_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Layout_T::makeSubStreamTail(): \"%m\", aborting\n")));
      return false;
    } // end IF
    prev_p->link (tail_p);
  } // end FOR

  // process any sub-distributors
  for (typename std::vector<typename inherited::iterator_base>::const_iterator iterator = sub_distributors_a.begin ();
       iterator != sub_distributors_a.end ();
       ++iterator)
    if (unlikely (!setup (*((*iterator).node))))
      return false;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
ACE_Module<ACE_SYNCH_USE, TimePolicyType>*
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::makeSubStreamTail () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::makeSubStreamTail"));

  TASK_T* tail_writer_p = NULL, *tail_reader_p = NULL;
  MODULE_T* module_p = NULL;

  ACE_NEW_NORETURN (tail_writer_p,
                    TAIL_WRITER_T ());
  ACE_NEW_NORETURN (tail_reader_p,
                    TAIL_READER_T ());
  if (unlikely (!tail_writer_p || !tail_reader_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete tail_writer_p; tail_writer_p = NULL;
    delete tail_reader_p; tail_reader_p = NULL;
    return NULL;
  } // end IF
  ACE_NEW_NORETURN (module_p,
                    MODULE_T (ACE_TEXT (STREAM_MODULE_TAIL_NAME),
                              tail_writer_p, tail_reader_p,
                              NULL,
                              ACE_Module_Base::M_DELETE));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete tail_writer_p; tail_writer_p = NULL;
    delete tail_reader_p; tail_reader_p = NULL;
    return NULL;
  } // end IF

  return module_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::find (MODULE_T* module_in,
                                           typename inherited::iterator_base& result_out) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::find"));

  // initialize return value(s)
  result_out = inherited::end ();

  // sanity check(s)
  ACE_ASSERT (module_in);

  for (ITERATOR_T iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
    if (unlikely (*iterator == module_in))
    {
      result_out = iterator;
      return true;
    } // end IF

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
void
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::prev (NODE_T& node_in,
                                           const std::string& name_in,
                                           Stream_ModuleList_t& list_inout) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::prev"));

  MODULE_T* prev_p = NULL;
  for (typename inherited::sibling_iterator iterator = inherited::begin (&node_in);
       iterator != inherited::end (&node_in);
       ++iterator)
  {
    if (unlikely (inherited::number_of_children (iterator)))
      prev (*(iterator.node),
            name_in,
            list_inout);

    if (unlikely (!ACE_OS::strcmp ((*iterator)->name (),
                                   ACE_TEXT (name_in.c_str ())) &&
                  prev_p))
      list_inout.push_back (prev_p);
    prev_p = (*iterator);
  } // end FOR
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename DistributorModuleType,
          typename TailTaskWriterType>
bool
Stream_Layout_T<ACE_SYNCH_USE,
                TimePolicyType,
                DistributorModuleType,
                TailTaskWriterType>::has_branch (NODE_T& node_in,
                                                 const std::string& branchName_in,
                                                 unsigned int& index_out) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Layout_T::has_branch"));

  // initialize return value(s)
  index_out = 0;

  // sanity check(s)
  if (unlikely (!is_distributor (node_in.data)))
    return false;

  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (node_in.data->writer ());
  ACE_ASSERT (idistributor_p);
  return idistributor_p->has (branchName_in,
                              index_out);
}
