/*
*继承此类的派生类不能被拷贝和赋值
*子类仍可以正常创建和销毁
*/
#pragma once
class noncopyable
{
public:
    noncopyable(const noncopyable &non) = delete;
    noncopyable &operator=(const noncopyable &non) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};