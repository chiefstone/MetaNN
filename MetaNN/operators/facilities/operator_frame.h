#pragma once

#include <cassert>
#include <type_traits>
#include <MetaNN/evaluate/facilities/eval_buffer.h>
#include <MetaNN/operators/facilities/organizer.h>

namespace MetaNN
{
template <typename TOpTag, typename THeadOp, typename...TOperands>
class Operator
{
    static_assert((std::is_same_v<RemConstRef<TOperands>, TOperands> && ...),
                  "TOperands is not available types");
public:
    using CategoryTag = OperCateCal<TOpTag, TOperands...>;
    using ElementType = typename OperElementType_<TOpTag, TOperands...>::type;
    using DeviceType = typename OperDeviceType_<TOpTag, TOperands...>::type;
    
public:
    Operator(TOperands&&... p_operands)
        : Operator(OperAuxParams<TOpTag, CategoryTag>{},
                   std::forward<TOperands>(p_operands)...)
    {}
    
    Operator(OperAuxParams<TOpTag, CategoryTag> auxParams,
             TOperands&&... p_operands)
        : m_auxParams(std::move(auxParams))
        , m_shapeInfo(m_auxParams, p_operands...)
        , m_operands({std::forward<TOperands>(p_operands)...})
    {}
    
    const auto& AuxParams() const
    {
        return m_auxParams;
    }
    
    const auto& Shape() const 
    {
        return m_shapeInfo.Shape();
    }
    
    bool operator== (const Operator& val) const
    {
        return (Shape() == val.Shape()) &&
               (m_operands == val.m_operands);
    }

    auto EvalRegister() const
    {
        if (!m_evalBuf.IsEvaluated())
        {
            using TOperSeqCont = typename OperSeq_<TOpTag>::type;
            
            using THead = SeqHead<TOperSeqCont>;
            using TTail = SeqTail<TOperSeqCont>;
            THead::template EvalRegister<TTail>(m_evalBuf, *this);
        }
        return m_evalBuf.ConstHandle();
    }
    
private:
    const OperAuxParams<TOpTag, CategoryTag> m_auxParams;
    const OperShapeInfo<TOpTag, CategoryTag> m_shapeInfo;
    const std::tuple<TOperands...> m_operands;
    
    using TPrincipal = PrincipalDataType<CategoryTag, ElementType, DeviceType>;
    EvalBuffer<TPrincipal> m_evalBuf;
};
}