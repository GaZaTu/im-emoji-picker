#pragma once

#include "emojis.hpp"
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPixmap>

class EmojiLabel : public QLabel {
  Q_OBJECT

public:
  explicit EmojiLabel(QWidget* parent = nullptr);
  explicit EmojiLabel(QWidget* parent, const Emoji& emoji);

  const Emoji& emoji() const;
  void setEmoji(const Emoji& emoji, int w = 24, int h = 24);

  bool highlighted() const;
  void setHighlighted(bool highlighted);

  bool hasRealEmoji() const;

signals:
  void mousePressed(QMouseEvent* ev);

protected:
  void mousePressEvent(QMouseEvent* ev) override;
  void mouseMoveEvent(QMouseEvent* ev) override;

private:
  Emoji _emoji;
  bool _hasRealEmoji = false;
  double _devicePixelRatio;

  QGraphicsDropShadowEffect* _shadowEffect = new QGraphicsDropShadowEffect();

  bool _useSystemEmojiFont = false;
  bool _useSystemEmojiFontWidthHeuristics = true;
};

bool fontSupportsEmoji(const QFontMetrics& metrics, const QString& text);
