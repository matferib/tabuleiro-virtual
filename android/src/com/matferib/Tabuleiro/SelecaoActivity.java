package com.matferib.Tabuleiro;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;

public class SelecaoActivity extends Activity implements View.OnClickListener {
  // Mensagem de comunicacao entre atividades.
  public final static String MENSAGEM_EXTRA = "com.matferib.Tabuleiro.MENSAGEM";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    ViewGroup grupo = new LinearLayout(this);
    // Endereco.
    endereco_ = new EditText(this);
    endereco_.setHint("endereço ou IP");
    grupo.addView(endereco_);
    // Botao.
    Button botao = new Button(this);
    botao.setText("Conectar");
    botao.setOnClickListener(this);
    grupo.addView(botao);
    // Finaliza.
    setContentView(grupo);
  }

  @Override
  protected void onPause() {
    super.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
  }

  @Override
  public void onClick(View v) {
    // Abre a activity do tabuleiro.
    Intent intencao = new Intent(this, TabuleiroActivity.class);
    intencao.putExtra(MENSAGEM_EXTRA, endereco_.getText().toString());
    startActivity(intencao);
  }

  // Membros.
  private EditText endereco_ = null;
}
